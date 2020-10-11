/*
 * @Descripttion:
 * @version:
 * @Author: zsj
 * @Date: 2020-10-10 11:07:08
 * @LastEditors: zsj
 * @LastEditTime: 2020-10-11 16:35:21
 */
#pragma once

#include <set>
#include <unordered_map>

#include "cache_status.h"
#include "sylar/mutex.h"
#include "sylar/util.h"

namespace sylar {

namespace ds {

/**
 * @brief 根据超时时间淘汰cache,使用set和unorderd_map
 * 1.使用一个set保存缓存的使用情况，超时时间小的放在头部，时间大的放在尾部，从头部淘汰
 * 2.使用一个unorderd_map保存缓存数据，便于查找是否命中缓存。
 */
template <class K, class V, class RWMutexType = sylar::RWMutex>
class TimedCache {
private:
    struct Item {
        Item(const K& k, const V& v, const uint64_t& t)
            : key(k), val(v), ts(t) {}

        K key;  //键
        mutable V val;  //值，mutable表示在使用const修饰的函数中也是可改的
        uint64_t ts;  //超时事件，ms

        //优先比较超时时间，相同后比较key
        bool operator<(const Item& oth) const {
            if (ts != oth.ts) {
                return ts < oth.ts;
            }
            return key < oth.key;
        }
    };


public:
    typedef Item item_type;
    typedef std::shared_ptr<TimedCache> ptr;
    typedef std::set<item_type> set_type;
    typedef std::unordered_map<K, typename set_type::iterator> map_type;
    typedef std::function<void(const K&, const V&)> prune_callback;

    TimedCache(size_t max_size = 0, size_t elasticity = 0,
               CacheStatus* status = nullptr)
        : m_maxSize(max_size), m_elasticity(elasticity), m_status(status) {
        if (m_status == nullptr) {
            m_status = new CacheStatus;
            m_statusOwner = true;
        }
    }
    ~TimedCache() {
        if (m_statusOwner && m_status) {
            delete m_status;
        }
    }

    void set(const K& k, const V& v, uint64_t expired) {
        m_status->incSet();
        typename RWMutexType::WriteLock lock(m_mutex);
        auto it = m_cache.find(k);
        if (it != m_cache.end()) {
            m_cache.erase(it);
        }

        auto sit = m_timed.insert(Item(k, v, expired + sylar::GetCurrentMS()));
        m_cache.insert(std::make_pair(k, sit.first));
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
        typename RWMutexType::ReadLock lock(m_mutex);
        auto it = m_cache.find(k);
        if (it == m_cache.end()) {
            return false;
        }

        v = it->second->val;
        lock.unlock();
        m_status->incHit();
        return true;
    }

    V get(const K& k) {
        m_status->incGet();
        typename RWMutexType::ReadLock lock(m_mutex);
        auto it = m_cache.find(k);
        if (it == m_cache.end()) {
            return V();
        }
        auto v = it->second->val;
        lock.unlock();
        m_status->incHit();
        return v;
    }

    bool del(const K& k) {
        m_status->incDel();
        typename RWMutexType::WriteLock lock(m_mutex);
        auto it = m_cache.find(k);
        if (it == m_cache.end()) {
            return false;
        }

        m_timed.erase(it->second);
        m_cache.erase(it);
        lock.unlock();
        m_status->incHit();
    }

    /**
     * @brief 判断缓存是否存在容器中，不存在直接返回，存在则修改超时时间
     * @param[in] k 键
     * @param[in] ts 超时时间
     * @return true:存在   false：不存在
     */
    bool expired(const K& k, const uint64_t& ts) {
        typename RWMutexType::WriteLock lock(m_mutex);
        auto it = m_cache.find(k);
        if (it == m_cache.end()) {
            return false;
        }

        uint64_t tts = ts + sylar::GetCurrentMS();
        if (it->second->ts == tts) {
            return true;
        }
        auto item = *(it->second);
        m_timed.erase(it->second);
        auto iit = m_timed.insert(item);
        it->second = iit.first;
        return true;
    }

    bool exists(const K& k) {
        typename RWMutexType::ReadLock lock(m_mutex);
        return m_cache.find(k) != m_cache.end();
    }

    size_t size() {
        typename RWMutexType::ReadLock lock(m_mutex);
        return m_cache.size();
    }

    bool clear() {
        typename RWMutexType::WriteLock lock(m_mutex);
        m_timed.clear();
        m_cache.clear();
        return true;
    }

    void setMaxSize(const size_t& v) { m_maxSize = v; }
    void setElasticity(const size_t& v) { m_elasticity = v; }
    size_t getMaxSize() const { return m_maxSize; }
    size_t getElasticity() const { return m_elasticity; }
    size_t getMaxAllowedSize() const { return m_maxSize + m_elasticity; }

    template <class F>
    void foreach (F& f) {
        typename RWMutexType::ReadLock lock(m_mutex);
        std::for_each(m_cache.begin(), m_cache.end(), f);
    }

    void setPruneCallback(prune_callback cb) { m_cb = cb; }

    std::string toStatusString() {
        std::stringstream ss;
        ss << (m_status ? m_status->toString() : "(no status)")
           << " total=" << size();
        return ss.str();
    }

    void setStatus(CacheStatus* v, bool owner = false) {
        if (m_statusOwner && m_status) {
            delete m_status;
        }
        m_status = v;
        m_statusOwner = owner;

        if (m_status = nullptr) {
            m_status = new CacheStatus;
            m_statusOwner = true;
        }
    }

    /**
     * @brief
     * 判断当前过期了的cache有多少，对于过期的cache，如果设置了回调函数，则调用
     * @param[in] 判定cache过期的时间标准，默认为当前时间，单位ms
     */
    size_t checkTimeout(const uint64_t& ts = sylar::GetCurrentMS()) {
        size_t size = 0;
        typename RWMutexType::WriteLock lock(m_mutex);
        for (auto it = m_timed.begin(); it != m_timed.end();) {
            if (it->ts <= ts) {
                if (m_cb) {
                    m_cb(it->key, it->val);
                }
                m_cache.erase(it->key);
                m_timed.erase(it++);
                ++size;
            } else {
                break;
            }
        }
        return size;
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

        size_t count = 0;
        while (m_cache.size() > m_maxSize) {
            auto it = m_timed.begin();
            if (m_cb) {
                m_cb(it->key, it->val);
            }
            m_cache.erase(it->key);
            m_timed.erase(it);
            ++count;
        }
        m_status->incPrune(count);
        return count;
    }


private:
    RWMutexType m_mutex;    //读写锁
    uint64_t m_maxSize;     //最大容量
    uint64_t m_elasticity;  //弹性容量
    CacheStatus* m_status;  //缓存状态
    map_type m_cache;  //缓存字典，用于查找缓存和判断缓存是否命中
    set_type m_timed;  //按照超时事件升序排列的缓存数据，优先淘汰头部缓存
    prune_callback m_cb;         //回调函数
    bool m_statusOwner = false;  //是否有状态信息
};                               // namespace ds

/**
 * @brief 最近最少使用缓存控制类，
 * - 使用vector模拟hashtable效果，每一个bucket的数据为TimedCache
 * 1.底层使用TimedCache作为hashtable每个bucket的数据结构，基础操作使用TimedCache的
 * 2.hash函数使用std::hash<K>
 * 3.整个数据结构为数组+集合+字典（vector+set+map)
 *
 * @tparam K 键类型
 * @tparam V 值类型
 * @tparam MutexType 互斥锁类型,默认使用sylar::Mutex
 * @tparam Hash hash类型，默认使用std::hash<K>
 */
template <class K, class V, class RWMutexType = sylar::RWMutex,
          class Hash = std::hash<K>>
class HashTimedCache {
public:
    typedef std::shared_ptr<HashTimedCache> ptr;
    typedef TimedCache<K, V, RWMutexType> cache_type;

    HashTimedCache(size_t bucket, size_t max_size, size_t elasticity)
        : m_bucket(bucket) {
        m_datas.resize(m_bucket);
        size_t pre_max_size = std::ceil(max_size * 1.0 / bucket);
        size_t pre_elasticity = std::ceil(elasticity * 1.0 / bucket);
        m_maxSize = pre_max_size * bucket;
        m_elasticity = pre_elasticity * bucket;

        for (size_t i = 0; i < bucket; i++) {
            m_datas[i] =
                new cache_type(pre_max_size, pre_elasticity, &m_status);
        }
    }
    ~HashTimedCache() {
        for (size_t i = 0; i < m_datas.size(); i++) {
            delete m_datas[i];
        }
    }
    void set(const K& k, const V& v, uint64_t expired) {
        m_datas[m_hash(k) % m_bucket]->set(k, v, expired);
    }

    bool expired(const K& k, const uint64_t& ts) {
        return m_datas[m_hash(k) % m_bucket]->expired(k, ts);
    }


    bool get(const K& k, V& v) {
        return m_datas[m_hash(k) % m_bucket]->get(k, v);
    }

    V get(const K& k) { return m_datas[m_hash(k) % m_bucket]->get(k); }

    bool del(const K& k) { return m_datas[m_hash(k) % m_bucket]->del(k); }

    bool exists(const K& k) { return m_datas[m_hash(k) % m_bucket]->exists(k); }

    size_t size() {
        size_t total = 0;
        for (auto& i : m_datas) {
            total += i->size();
        }
        return total;
    }

    bool empty() {
        for (auto& i : m_datas) {
            if (!i->empty()) {
                return false;
            }
        }
        return true;
    }

    void clear() {
        for (auto& i : m_datas) {
            i->clear();
        }
    }

    size_t getMaxSize() const { return m_maxSize; }

    size_t getElasticity() const { return m_elasticity; }

    size_t getMaxAllowedSize() const { return m_maxSize + m_elasticity; }

    size_t getBucket() const { return m_bucket; }

    void setMaxSize(const size_t& v) {
        size_t pre_max_size = std::ceil(v * 1.0 / m_bucket);
        m_maxSize = pre_max_size * m_bucket;
        for (auto& i : m_datas) {
            i->setMaxSize(pre_max_size);
        }
    }

    void setElasticity(const size_t& v) {
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

    CacheStatus* getStatus() { return &m_status; }

    std::string toStatusString() {
        std::stringstream ss;
        ss << m_status.toString() << " total=" << size();
        return ss.str();
    }

    size_t checkTimeout(const uint64_t& ts = sylar::GetCurrentMS()) {
        size_t size = 0;
        for (auto& i : m_datas) {
            size += i->checkTimeout(ts);
        }
        return size;
    }

private:
    std::vector<cache_type*> m_datas;  // hashtable
    size_t m_maxSize;                  //最大总容量
    size_t m_bucket;                   //桶数量
    size_t m_elasticity;               //总弹性值
    Hash m_hash;                       // hash函数
    CacheStatus m_status;              //缓存状态

private:
};

}  // namespace ds

}  // namespace sylar