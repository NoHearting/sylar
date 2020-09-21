/*
 * @Descripttion: 
 *  文件句柄管理类
 * @version: 
 * @Author: zsj
 * @Date: 2020-06-13 15:19:28
 * @LastEditors: zsj
 * @LastEditTime: 2020-07-07 22:37:28
 */ 
#pragma once

#include<vector>
#include<memory>
#include"singleton.h"
#include"thread.h"
#include"iomanager.h"

namespace sylar{

/**
 * @brief 文件句柄上下文类
 * 
 * @detail 管理文件句柄类型：是否是socket，是否阻塞，是否关闭，读写超时时间
 */ 
class FdCtx : std::enable_shared_from_this<FdCtx>{
public:
    typedef std::shared_ptr<FdCtx> ptr;
    
    /**
     * @brief 通过文件句柄构建FdCtx
     * 
     * @param[in] fd 文件句柄
     */ 
    FdCtx(int fd);
    ~FdCtx();

    

    /**
     * @brief 是否初始化完成
     */ 
    bool isInit()const {return m_isInit;}

    /**
     * @brief 是否为socket
     * 
     */ 
    bool isSocket() const {return m_isSocket;}

    /**
     * @brief 是否已经关闭
     * 
     */ 
    bool isClose() const{return m_isClosed;}
    bool close();

    /**
     * @brief 设置用户是否非阻塞
     * @param[in] v 是否阻塞
     */ 
    void setUserNonblock(bool v){m_userNonblock = v;}

    /**
     * @brief 获取用户是否非阻塞
     * 
     */ 
    bool getUserNonblock()const {return m_userNonblock;}

    /**
     * @brief 设置系统非阻塞
     * @param[in] v 是否阻塞
     */ 
    void setSysNonblock(bool v){m_sysNonblock = v;}

    /**
     * @brief 获取系统是否非阻塞
     * 
     */ 
    bool getSysNonblock()const {return m_sysNonblock;}

    /**
     * @brief 设置超时时间
     * @param[in] type 类型为 SO_RCVTIMEO(读超时)，SO_SNDTIMEO(写超时)
     * @param[in] v 超时时间(ms)
     */ 
    void setTimeout(int type,uint64_t v);

    /**
     * @brief 获取超时时间
     * @param[in] type 类型为 SO_RCVTIMEO(读超时)，SO_SNDTIMEO(写超时)
     * @return 超时时间(ms)
     */ 
    uint64_t getTimeout(int type);

private:
    /**
     * @brief 初始化
     * 
     * 判断文件描述符是否打开
     *  1. 打开，设置该文件描述符为非阻塞，置m_sysNonblock = true
     *  2. 未打开，设置所有标志位为false
     */ 
    bool init();    
private:
    /// 是否初始化
    bool m_isInit: 1;

    /// 是否socket
    bool m_isSocket: 1;

    /// 是否hook非阻塞
    bool m_sysNonblock: 1;

    /// 是否用户主动设置非阻塞
    bool m_userNonblock: 1;

    /// 是否关闭
    bool m_isClosed: 1;

    /// 文件句柄
    int m_fd;

    /// 读超时时间
    uint64_t m_recvTimeout;

    /// 写超时时间
    uint64_t m_sendTimeout;
    sylar::IOManager * m_iomanager;
};

/**
 * @brief 文件句柄管理类
 */ 
class FdManager{
public:
    typedef RWMutex RWMutexType;
    FdManager();

    /**
     * @brief 获取/创建文件句柄对象FdCtx
     * 
     * @param[in] fd 文件句柄
     * @param[in] auto_create 是否自动创建文件句柄
     * @return 返回一个文件句柄类对象FdCtx::ptr
     */ 
    FdCtx::ptr get(int fd,bool auto_create = false);

    /**
     * @brief 删除一个文件句柄类对象
     * @param[in] fd 文件描述符
     */ 
    void del(int fd);
private:

    /// 读写锁
    RWMutexType m_mutex;

    /// 文件句柄集合
    std::vector<FdCtx::ptr> m_datas;
};

/// 文件句柄单例
typedef Singleton<FdManager> FdMgr;

}