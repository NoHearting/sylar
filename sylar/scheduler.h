/*
 * @Descripttion: 
 *  1.设置当前线程的scheduler
 *  2.设置当前线程的run，fiber
 *  3.协程调度循环
 *      1. 协程消息队列是否有任务
 *      2. 无任务执行，执行idle
 * @version: 
 * @Author: zsj
 * @Date: 2020-06-10 13:13:19
 * @LastEditors: zsj
 * @LastEditTime: 2020-06-12 16:13:08
 */ 
#ifndef __SYLAR_SCHEDULER_H__
#define __SYLAR_SCHEDULER_H__

#include<memory>
#include<vector>
#include<list>
#include<functional>
#include<atomic>

#include"thread.h"
#include"fiber.h"

namespace sylar{

class Scheduler{

public:
    typedef std::shared_ptr<Scheduler> ptr;
    typedef Mutex MutexType;

    Scheduler(size_t threads = 1,bool use_caller = true,const std::string & name = "");
    virtual ~Scheduler();

    //Normal
    void start();
    void stop();

    template<class FiberOrCb>
    void schedule(FiberOrCb fc,int thread = -1){
        bool need_tickle = false;
        {
            MutexType::Lock lock(m_mutex);
            need_tickle = scheduleNoLock(fc,thread);
        }
        if(need_tickle){
            tickle();
        }

    }

    template<class InputIterator>
    void schedule(InputIterator begin,InputIterator end){
        bool need_tickle = false;
        {
            MutexType::Lock lock(m_mutex);
            while(begin != end){
                need_tickle = scheduleNoLock(&*begin,-1) || need_tickle;
                ++begin;
            }
        }
        if(need_tickle){
            tickle();
        }
    }

    //Getter and Setter
    const std::string & getName()const{return m_name;}

    //Static
    static Scheduler * GetThis();
    static Fiber * GetMainFiber();
protected:
    virtual void tickle();
    void run();
    virtual bool stopping();
    virtual void idle();

    void setThis();

    bool hasIdleThread() {return m_idleThreadCount > 0;}
private:
    template<class FiberOrCb>
    bool scheduleNoLock(FiberOrCb fc,int thread){
        bool nedd_tickle = m_fibers.empty();
        FiberAndThread ft(fc,thread);
        if(ft.fiber || ft.cb){
            m_fibers.push_back(ft);
        }
        return nedd_tickle;
    }

private:
    struct FiberAndThread{
        Fiber::ptr fiber;
        std::function<void()> cb;
        int thread;

        FiberAndThread(Fiber::ptr f,int thr):fiber(f),thread(thr){}
        FiberAndThread(Fiber::ptr * f,int thr):
            thread(thr){
            fiber.swap(*f);
        }

        FiberAndThread(std::function<void()> f,int thr)
            :cb(f),thread(thr){
                
        }

        FiberAndThread(std::function<void()> * f,int thr):
            thread(thr){
            cb.swap(*f);
        }
        FiberAndThread()
            :thread(-1){

        }

        void reset(){
            fiber = nullptr;
            cb = nullptr;
            thread = -1;
        }
    };

private:
    MutexType m_mutex;
    std::vector<Thread::ptr> m_threads;
    std::list<FiberAndThread> m_fibers;  //协程队列，需要执行的协程
    Fiber::ptr m_rootFiber;  //主协程
    std::string m_name;

protected:
    std::vector<int> m_threadIds;
    size_t m_threadCount = 0;   //总线程数量
    std::atomic<size_t> m_activeThreadCount = {0}; //当前活跃线程数量
    std::atomic<size_t> m_idleThreadCount = {0};   //当前空闲线程数量
    bool m_stoping = true;
    bool m_autoStop = false;
    int m_rootThread = 0;  //主线程id


};

}

#endif