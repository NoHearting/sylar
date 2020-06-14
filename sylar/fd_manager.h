/*
 * @Descripttion: 
 * @version: 
 * @Author: zsj
 * @Date: 2020-06-13 15:19:28
 * @LastEditors: zsj
 * @LastEditTime: 2020-06-13 16:30:38
 */ 
#pragma once

#include<vector>
#include<memory>
#include"singleton.h"
#include"thread.h"
#include"iomanager.h"

namespace sylar{

class FdCtx : std::enable_shared_from_this<FdCtx>{
public:
    typedef std::shared_ptr<FdCtx> ptr;
    
    FdCtx(int fd);
    ~FdCtx();

    bool init();
    bool isInit()const {return m_isInit;}
    bool isSocket() const {return m_isSocket;}
    bool isClose() const{return m_isClosed;}
    bool close();

    void setUserNonblock(bool v){m_userNonblock = v;}
    bool getUserNonblock()const {return m_userNonblock;}

    void setSysNonblock(bool v){m_sysNonblock = v;}
    bool getSysNonblock()const {return m_sysNonblock;}

    void setTimeout(int type,uint64_t v);
    uint64_t getTimeout(int type);
private:
    bool m_isInit: 1;
    bool m_isSocket: 1;
    bool m_sysNonblock: 1;
    bool m_userNonblock: 1;
    bool m_isClosed: 1;
    int m_fd;

    uint64_t m_recvTimeout;
    uint64_t m_sendTimeout;
    sylar::IOManager * m_iomanager;
};


class FdManager{
public:
    typedef RWMutex RWMutexType;
    FdManager();

    FdCtx::ptr get(int fd,bool auto_create = false);
    void del(int fd);
private:
    RWMutexType m_mutex;
    std::vector<FdCtx::ptr> m_datas;
};

/// 文件句柄单例
typedef Singleton<FdManager> FdMgr;

}