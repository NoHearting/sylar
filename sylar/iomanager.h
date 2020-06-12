/*
 * @Descripttion: 
 * @version: 
 * @Author: zsj
 * @Date: 2020-06-10 20:37:28
 * @LastEditors: zsj
 * @LastEditTime: 2020-06-12 20:02:33
 */ 
#ifndef __SYLAR_IOMANAGER_H__
#define __SYLAR_IOMANAGER_H__

#include"scheduler.h"
#include"time.h"
#include"timer.h"

namespace sylar{

class IOManager : public Scheduler,public TimerManager{
public:
    typedef std::shared_ptr<IOManager> ptr;
    typedef RWMutex RWMutexType;

    enum Event{
        NONE = 0x0,
        READ = 0x1,   //EPOLLOUT
        WRITE = 0x4   //EPOLLIN
    };

public:
    IOManager(size_t threads = 1,bool use_caller = true,const std::string & name = "");
    ~IOManager();

    /**
     * @brief 添加一个事件
     *  - 1是成功，0是重试，-1是失败
     */ 
    int addEvent(int fd,Event event,std::function<void()> cb = nullptr);
    bool delEvent(int fd,Event event);
    bool cancelEvent(int fd,Event event);

    bool cancelAll(int fd);

    //static
    static IOManager* GetThis();

protected:
    virtual void tickle() override;
    virtual bool stopping() override;
    bool stopping(uint64_t & timeout);
    virtual void idle() override;
    void onTimerInsertedAtFront() override;
    void contextResize(size_t size);
private:
    struct FdContext{
        
        typedef Mutex MutexType;

        struct EventContext{
            Scheduler * scheduler = nullptr;      //事件执行的scheduler
            Fiber::ptr fiber;           //事件协程
            std::function<void()> cb;   //事件的回调函数
        };

        EventContext & getContext(Event event);
        void resetContext(EventContext & ctx);
        void triggerEvent(Event event);
        int fd = 0;                     //事件关联的文件描述符
        EventContext read;          //读事件
        EventContext write;         //写事件
        Event events = NONE;      //已注册的事件

        MutexType mutex;

    };

private:
    int m_epfd = 0;   //epoll文件描述符
    int m_tickleFds[2];  // 0 为写入   1 为读出


    std::atomic<size_t> m_pendingEventCount = {0};  //等待执行的事件数量
    RWMutexType m_mutex;
    std::vector<FdContext *>m_fdContexts;

};


}


#endif