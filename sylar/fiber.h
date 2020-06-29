/*
 * @Descripttion: 
 *  每个线程有一个主协程，主协程可以新建子协程，可以在主协程和子协程之间的切换
 * @version: 
 * @Author: zsj
 * @Date: 2020-06-08 19:40:53
 * @LastEditors: zsj
 * @LastEditTime: 2020-06-29 20:30:59
 */ 
#ifndef __SYLAR_FIBER_H__
#define __SYLAR_FIBER_H__

#include<ucontext.h>
#include<memory>
#include<functional>

// #include"thread.h"

namespace sylar{

class Scheduler;
class Fiber : public std::enable_shared_from_this<Fiber>{
friend class Scheduler;
public:
    typedef std::shared_ptr<Fiber> ptr;

    enum State{
        INIT,
        HOLD,
        EXEC,
        TERM,
        READY,
        EXECPT
    };

private:
    Fiber();
public:
    Fiber(std::function<void()> cb,size_t stacksize = 0,bool use_caller = false);
    ~Fiber();

    //重置协程函数，并重置状态
    void reset(std::function<void()> cb);

    //切换到当前协程执行
    void swapIn();

    //切换到后台执行
    void swapOut();

    void call();
    void back();

    uint64_t getId()const{return m_id;}
    Fiber::State getState()const{return m_state;}
    void setState(Fiber::State state){m_state = state;}
public:

    //设置当前协程
    static void SetThis(Fiber * f);

    //返回当前协程
    static Fiber::ptr GetThis();

    //协程切换到后台，并且设置为Ready状态
    static void YieldToReady();

    //协程切换到后台，并设置为Hold状态
    static void YieldToHold();

    //总协程数
    static uint64_t TotalFibers();

    static void CallerMainFunc();
    static void MainFunc();

    static uint64_t GetFiberId();
private:
    uint64_t m_id = 0;
    uint32_t m_stacksize = 0;
    State m_state = INIT;
    
    ucontext_t m_ctx;
    void * m_stack = nullptr;

    std::function<void()> m_cb;
};


}


#endif