/*
 * @Descripttion: 
 * @version: 
 * @Author: zsj
 * @Date: 2020-06-29 16:28:54
 * @LastEditors: zsj
 * @LastEditTime: 2020-07-05 20:53:05
 */ 
#pragma once


#include "mutex.h"
#include "singleton.h"
#include "log.h"
#include "iomanager.h"

static sylar::Logger::ptr g_logger = SYLAR_LOG_NAME("system");

namespace sylar
{
    

class WorkerGroup : Noncopyable,
    public std::enable_shared_from_this<WorkerGroup>{
public:
    typedef std::shared_ptr<WorkerGroup> ptr;
    static WorkerGroup::ptr Create(uint32_t batch_size,
                Scheduler* s = Scheduler::GetThis()){
        return std::make_shared<WorkerGroup>(batch_size,s);
    }

    WorkerGroup(uint32_t batch_size,Scheduler * s = Scheduler::GetThis());

    ~WorkerGroup();

    void schedule(std::function<void()>cb,int thread = -1);
    void waitAll();

private:
    void doWork(std::function<void()> cb);

private:
    uint32_t m_batchSize;
    bool m_finish;
    Scheduler * m_shceduler;
    FiberSemaphore m_sem;
};

class WorkerManager{
public:
    WorkerManager();
    void add(Scheduler::ptr s);

    Scheduler::ptr get(const std::string & name);
    IOManager::ptr getAsIOManager(const std::string & name);

    template<class FiberOrCb>
    void schedule(const std::string & name,FiberOrCb fc,int thread = -1){
        auto s = get(name);
        if(s){
            s->schedule(fc,thread);
        }
        else{
            SYLAR_LOG_ERROR(g_logger) << "schedule name=" << name << "not exists";
        }
    }

    template<class Iter>
    void schedule(const std::string & name,Iter begin,Iter end){
        auto s = get(name);
        if(s){
            s->schedule(begin,end);
        }
        else{
            SYLAR_LOG_ERROR(g_logger) << "schedule name=" << name << " not exists";
        }

    }

    bool init();
    void stop();

    bool isStoped() const {return m_stop;}
    std::ostream & dump(std::ostream & os);

    uint32_t getCount();

private:
    std::map<std::string,std::vector<Scheduler::ptr>>m_datas;
    bool m_stop;
};

typedef sylar::Singleton<WorkerManager> WorkerMgr;

} // namespace sylar


