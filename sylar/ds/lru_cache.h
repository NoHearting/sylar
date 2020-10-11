/*
 * @Descripttion:
 * @version:
 * @Author: zsj
 * @Date: 2020-10-09 15:19:54
 * @LastEditors: zsj
 * @LastEditTime: 2020-10-11 16:38:22
 */
#pragma once

#include <algorithm>
#include <atomic>
#include <list>
#include <sstream>
#include <unordered_map>
#include <vector>

#include "cache_status.h"
#include "sylar/log.h"
#include "sylar/mutex.h"

static sylar::Logger::ptr system_logger = SYLAR_LOG_NAME("system");
namespace sylar {
namespace ds {

/**
 * @brief 最近最少使用缓存控制类，使用list和unorderd_map
 * 1.使用一个list来保存缓存的使用情况，使用最频繁的cache放链表头部，使用最不频繁的放链表尾部
 *   1.1 当缓存容量达到最大时，淘汰最近最少使用的cache（list尾部的cache）
 * 2.使用一个unorderd_map来保存缓存的数据，方便查找是否命中当前缓存。
 *   2.1 命中则将list中对应的cache移到头部
 * @tparam K 键类型
 * @tparam V 值类型
 * @tparam MutexType 互斥锁类型
 */
template <class K, class V, class MutexType = sylar::Mutex>
class LruCache {
public:
    typedef std::shared_ptr<LruCache> ptr;
    typedef std::pair<K, V> item_type;
    typedef std::list<item_type> list_type;
    typedef std::unordered_map<K, typename list_type::iterator> map_type;
    typedef std::function<void(const K&, const V&)> prune_callback;

    LruCache(size_t max_size = 0, size_t elasticity = 0,
             CacheStatus* status = nullptr)
        : m_maxSize(max_size), m_elasticity(elasticity) {
        m_status = status;
        if (m_status == nullptr) {
            m_status = new CacheStatus;
            m_statusOwner = true;
        }
    }

    ~LruCache() {
        if (m_statusOwner && m_status) {
            delete m_status;
        }
    }

    void set(const K& k, const V& v) {
        m_status->incSet();
        typename MutexType::Lock lock(m_mutex);
        auto it = m_cache.find(k);
        if (it != m_cache.end()) {
            // SYLAR_LOG_DEBUG(system_logger) << "find key: " << k;
            it->second->second = v;
            m_keys.splice(m_keys.begin(), m_keys, it->second);
            return;
        }
        m_keys.emplace_front(std::make_pair(k, v));  //在list头部添加数据
        m_cache.insert(std::make_pair(k, m_keys.begin()));
        // SYLAR_LOG_DEBUG(system_logger) << "set key: " << k;
        prune();
    }


    /**
     * @brief 获取cache中key对应的值
     * @param[in] k 键
     * @param[out] v 值
     * @return bool 获取成功与否
     */
    bool get(const K& k, V& v) {
        m_status->incGet();
        typename MutexType::Lock lock(m_mutex);
        auto it = m_cache.find(k);
        if (it == m_cache.end()) {
            return false;
        }
        m_keys.splice(m_keys.begin(), m_keys, it->second);
        v = it->second->second;
        lock.unlock();
        m_status->incHit();
        return true;
    }

    V get(const K& k) {
        m_status->incGet();
        typename MutexType::Lock lock(m_mutex);
        auto it = m_cache.find(k);
        if (it == m_cache.end()) {
            return false;
        }
        // A.splcie(pos,B,Bpos)  将B的Bpos位置上的元素移到A的Apos上
        m_keys.splice(m_keys.begin(), m_keys, it->second);
        auto v = it->second->second;
        lock.unlock();
        m_status->incHit();
        return v;
    }

    bool del(const K& k) {
        m_status->incDel();
        typename MutexType::Lock lock(m_mutex);
        auto it = m_cache.find(k);
        if (it == m_cache.end()) {
            return false;
        }
        m_keys.erase(it->second);
        m_cache.erase(it);
        return true;
    }

    bool exists(const K& k) {
        typename MutexType::Lock lock(m_mutex);
        return m_cache.find(k) != m_cache.end();
    }

    size_t size() {
        typename MutexType::Lock lock(m_mutex);
        return m_cache.size();
    }

    bool empty() {
        typename MutexType::Lock lock(m_mutex);
        return m_cache.empty();
    }

    bool clear() {
        typename MutexType::Lock lock(m_mutex);
        m_cache.clear();
        m_keys.clear();
        return true;
    }

    void setMaxSize(const size_t& v) { m_maxSize = v; }
    void setElasticity(const size_t& v) { m_elasticity = v; }

    size_t getMaxSize() const { return m_maxSize; }
    size_t getElasticity() const { return m_elasticity; }
    size_t getMaxAllowedSize() const { return m_maxSize + m_elasticity; }


    /**
     * @brief 遍历处理缓存数据
     * @param[in] f 模板参数，接受一个处理函数，用于遍历处理缓存容器中的所有缓存
     */
    template <class F>
    void foreach (F& f) {
        typename MutexType::Lock lock(m_mutex);
        std::for_each(m_cache.begin(), m_cache.end(), f);
    }

    void setPruneCallback(prune_callback cb) { m_cb = cb; }

    /**
     * @brief 返回状态信息
     * @return std::string
     */
    std::string toStatusString() {
        std::stringstream ss;
        ss << (m_status ? m_status->toString() : "(no status)")
           << " total=" << size();
        return ss.str();
    }

    CacheStatus* getStatus() const { return m_status; }

    void setStatus(CacheStatus* v, bool owner = false) {
        if (m_statusOwner && m_status) {
            delete m_status;
        }
        m_status = v;
        m_statusOwner = owner;
        if (m_status == nullptr) {
            m_status = new CacheStatus;
            m_statusOwner = true;
        }
    }

protected:
    /**
     * @brief
     * 保证活跃缓存数量小于设置的最大值，如果超过最大值，则淘汰最近未使用的缓存
     */
    size_t prune() {
        if (m_maxSize == 0 || m_cache.size() < getMaxAllowedSize()) {
            return 0;
        }
        // SYLAR_LOG_DEBUG(system_logger) << " exec prune - "
        //    << "cache size: " << m_cache.size();
        size_t count = 0;
        while (m_cache.size() > m_maxSize) {
            auto& back = m_keys.back();
            if (m_cb) {
                m_cb(back.first, back.second);
            }
            m_cache.erase(back.first);
            m_keys.pop_back();
            ++count;
        }
        m_status->incPrune(count);
        return count;
    }

private:
    MutexType m_mutex;  //互斥量
    map_type m_cache;   //缓存的存储容器，查找缓存时使用
    //缓存的键值列表，保存缓存的键值，使用频繁的缓存放在链表头，不频繁的放在链表尾
    list_type m_keys;
    size_t m_maxSize;                 //缓存的最大数量
    size_t m_elasticity;              //缓存允许的弹性数量
    prune_callback m_cb;              //淘汰缓存时调用的回调函数
    CacheStatus* m_status = nullptr;  //缓存容器的状态信息
    bool m_statusOwner = false;       //有无状态信息


};  // LruCache
/**
 * @brief 根据超时时间确定缓存控制类，
 * - 使用vector模拟hashtable效果，每一个bucket的数据为LruCache
 * 1. 底层使用LruCache作为hashtable每个bucket的数据结构，基础操作使用LruCache的
 * 2. hash函数使用std::hash<K>
 * 3. 整个数据结构为数组+链表+字典（vector+list+map)
 *
 * @tparam K 键类型
 * @tparam V 值类型
 * @tparam MutexType 互斥锁类型,默认使用sylar::RWMutex
 * @tparam Hash hash类型，默认使用std::hash<K>
 */
template <class K, class V, class MutexType = sylar::Mutex,
          class Hash = std::hash<K>>
class HashLruCache {
public:
    typedef std::shared_ptr<HashLruCache> ptr;
    typedef LruCache<K, V, MutexType> cache_type;
    HashLruCache(size_t bucket, size_t max_size, size_t elasticity)
        : m_bucket(bucket) {
        m_datas.resize(bucket);
        // ceil向上取整
        size_t pre_max_size = std::ceil(max_size * 1.0 / bucket);
        size_t pre_elasticity = std::ceil(elasticity * 1.0 / bucket);
        m_maxSize = pre_max_size * bucket;
        m_elasticity = pre_elasticity * bucket;

        //给每个bucket分配相同的内存块最大值（max_size)和伸缩值(elasticity)和缓存状态（status）
        //所有bucket共用一个缓存状态（m_status)
        for (size_t i = 0; i < bucket; i++) {
            m_datas[i] =
                new cache_type(pre_max_size, pre_elasticity, &m_status);
        }
    }

    ~HashLruCache() {
        for (size_t i = 0; i < m_datas.size(); i++) {
            delete m_datas[i];
        }
    }

    /**
     * @brief 设置缓存容器的访问命中，
     * - 根据std::hash返回的值确定调用哪一个bucket中的LruCache的set函数
     * @param[in] k 键
     * @param[in] v 值
     */
    void set(const K& k, const V& v) {
        auto pos = m_hash(k) % m_bucket;
        SYLAR_LOG_DEBUG(system_logger)
            << "set bucket " << pos << " key=" << k << " value=" << v;
        m_datas[m_hash(k) % m_bucket]->set(k, v);
    }

    bool get(const K& k, V& v) {
        return m_datas[m_hash(k) % m_bucket]->get(k, v);
    }

    V get(const K& k) { return m_datas[m_hash(k) % m_bucket]->get(k); }

    bool del(const K& k) { return m_datas[m_hash(k) % m_bucket]->del(k); }

    bool exists(const K& k) { return m_datas[m_hash(k) % m_bucket]->exists(k); }

    /**
     * @brief 当前缓存容器中活跃的缓存数量，计算所有bucket中的cache
     * @return size_t
     */
    size_t size() {
        size_t total = 0;
        for (auto& i : m_datas) {
            total += i->size();
        }
        return total;
    }

    /**
     * @brief 判断当前缓存容器是否为空，只要有一个活跃的cache就非空
     * @return bool
     */
    bool empty() {
        for (auto& i : m_datas) {
            if (!i->empty()) {
                return false;
            }
        }
        return true;
    }

    /**
     * @brief 清空缓存容器中的数据
     */
    void clear() {
        for (auto& i : m_datas) {
            i->clear();
        }
    }

    size_t getMaxSize() const { return m_maxSize; }
    size_t getElasticity() const { return m_elasticity; }
    size_t getMaxAllowedSize() const { return m_maxSize + m_elasticity; }
    size_t getBucket() const { return m_bucket; }

    /**
     * @brief 设置最大的缓存容量，分别设置bucket中的LruCache的最大容量
     * @param[in] v 最大容量总和
     */
    void setMaxSize(const size_t& v) {
        size_t pre_max_size = std::ceil(v * 1.0 / m_bucket);
        m_maxSize = pre_max_size * m_bucket;
        for (auto& i : m_datas) {
            i->setMaxSize(pre_max_size);
        }
    }

    /**
     * @brief 设置弹性值，分别设置bucket中的LruCache的弹性值
     * @param[in] v 弹性值总和
     */
    void setElassticity(const size_t& v) {
        size_t pre_elasticity = std::ceil(v * 1.0 / m_bucket);
        m_elasticity = pre_elasticity * m_bucket;
        for (auto& i : m_datas) {
            i->setElasticity(pre_elasticity);
        }
    }

    template <class F>
    void foreach (F& f) {
        for (auto& i : m_datas) {
            i->foreach (f);
        }
    }

    void setPruneCallback(typename cache_type::prune_callback cb) {
        for (auto& i : m_datas) {
            i->setPruneCallback(cb);
        }
    }

    std::string toStatusString() {
        std::stringstream ss;
        ss << m_status.toString() << " total=" << size();
        return ss.str();
    }

    CacheStatus* getStatus() { return &m_status; }

private:
    std::vector<cache_type*> m_datas;
    size_t m_maxSize;
    size_t m_bucket;
    size_t m_elasticity;
    Hash m_hash;
    CacheStatus m_status;
};

}  // namespace ds

}  // namespace sylar
