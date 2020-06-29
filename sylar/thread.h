/*
 * @Descripttion: 
 * @version: 
 * @Author: zsj
 * @Date: 2020-06-08 10:50:34
 * @LastEditors: zsj
 * @LastEditTime: 2020-06-29 17:04:36
 */ 
#ifndef __SYLAR__THREAD_H__
#define __SYLAR__THREAD_H__

//使用pthread
#include<pthread.h>
#include<thread>
#include<functional>
#include<memory>
#include<string>
#include<stdint.h>
#include<semaphore.h>
#include<atomic>
#include<iostream>

#include "mutex.h"

namespace sylar{

/**
 * @brief 线程类
 */ 
class Thread{
public:
    typedef std::shared_ptr<Thread> ptr;
    Thread(std::function<void()> cb,const std::string & name);
    ~Thread();

    pid_t getId()const {return m_id;}
    void setId(pid_t id){m_id = id;}
    const std::string & getName() const {return m_name;}

    void join();
    

    static Thread * GetThis();
    static const std::string GetName();
    static void SetName(const std::string & name);
    
private:
    Thread(const Thread & ) = delete;
    Thread(const Thread &&) = delete;
    Thread & operator=(const Thread & ) = delete;
    static void * run(void * arg);
private:
    pid_t m_id = -1;
    pthread_t m_thread = 0;
    std::function<void()> m_cb;
    std::string m_name;

    Semaphore m_semaphore;
};

}

#endif