/*
 * @Descripttion: 
 * @version: 
 * @Author: zsj
 * @Date: 2020-06-08 11:19:30
 * @LastEditors: zsj
 * @LastEditTime: 2020-06-29 20:50:06
 */ 
#include"thread.h"
#include"log.h"
#include"util.h"



namespace sylar{


static thread_local Thread* t_thread = nullptr;
static thread_local std::string t_thread_name = "UNKNOW";

static sylar::Logger::ptr g_logger = SYLAR_LOG_NAME("system");





//---------------------------------- Thread ------------------------------

Thread::Thread(std::function<void()> cb,const std::string & name):m_cb(cb){
    if(name.empty()){
        m_name = "UNKOWN";
    }else{
        m_name = name;
        // t_thread_name = name;
        // Thread::SetName(name);
    }

    int rt = pthread_create(&m_thread,nullptr,&Thread::run,this);
    if(rt){
        SYLAR_LOG_ERROR(g_logger) << "pthread_create thread fail,rt = "<<rt
            << " name = "<<m_name;
            throw std::logic_error("pthread_create error");
    }
    m_semaphore.wait();
}
Thread::~Thread(){
    if(m_thread){
        pthread_detach(m_thread);
    }
}


void Thread::join(){
    if(m_thread){
        int rt = pthread_join(m_thread,nullptr);
        if(rt){
            SYLAR_LOG_ERROR(g_logger) << "pthread_join thread fail,rt = "<<rt
            << " name = "<<m_name;
            throw std::logic_error("pthread_join error");
        }
        m_thread = 0;
    }
}


void * Thread::run(void * arg)
{
    
    Thread * thread = (Thread*)arg;
    t_thread = thread;
    t_thread->m_semaphore.notify();
    thread->setId(sylar::GetThreadId());
    t_thread_name = thread->getName();
    pthread_setname_np(pthread_self(),thread->getName().substr(0,15).c_str());
    std::function<void()> cb;
    cb.swap(thread->m_cb);
    
    cb();
    return 0;
}

Thread * Thread::GetThis(){
    // SetName(m_name);
    return t_thread;
}
const std::string Thread::GetName(){
    return t_thread_name;
}
void Thread::SetName(const std::string & name)
{
    if(name.empty()){
        return;
    }

    if(t_thread){
        // t_thread->SetName(name);
        t_thread->m_name = name;
    }
    t_thread_name = name;
}

}