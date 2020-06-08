/*
 * @Descripttion: 
 * @version: 
 * @Author: zsj
 * @Date: 2020-06-08 10:50:34
 * @LastEditors: zsj
 * @LastEditTime: 2020-06-08 17:08:03
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

namespace sylar{

class Semaphore{
public:
    Semaphore(uint32_t conut = 0);
    ~Semaphore();

    void wait();
    void notify();
private:
    Semaphore(const Semaphore &) = delete;
    Semaphore(const Semaphore &&) = delete;
    Semaphore & operator=(const Semaphore & ) = delete;

private:
    sem_t m_semaphore;
};


template<typename T>
class ScopedLockImpl{
public:
    ScopedLockImpl(T & mutex) : m_mutex(mutex){
        m_mutex.lock();
        m_locked = true;
    }

    ~ScopedLockImpl(){
        m_mutex.unlock();
    }

    void lock(){
        if(!m_locked){
            m_mutex.lock();
            m_locked = true;
        }
    }

    void unlock(){
        if(m_locked){
            m_mutex.unlock();
            m_locked = false;
        }
    }
private:
    T & m_mutex;
    bool m_locked;
};


class Mutex{
public:

    typedef ScopedLockImpl<Mutex> Lock;

    Mutex(){
        pthread_mutex_init(&m_mutex,nullptr);
    }

    ~Mutex(){
        pthread_mutex_destroy(&m_mutex);
    }

    void lock(){
        pthread_mutex_lock(&m_mutex);
    }

    void unlock(){
        pthread_mutex_unlock(&m_mutex);
    }
private:
    pthread_mutex_t m_mutex;
};

//! 测试使用
class NullMutex{
public:
    typedef ScopedLockImpl<NullMutex> Lock;
    void lock() {}
    void unlock() {}
};

class SpinLock{
public:
    typedef ScopedLockImpl<SpinLock> Lock;

    SpinLock(){
        pthread_spin_init(&m_mutex,0);
    }
    ~SpinLock(){
        pthread_spin_destroy(&m_mutex);
    }

    void lock(){
        pthread_spin_lock(&m_mutex);
    }

    void unlock(){
        pthread_spin_unlock(&m_mutex);
    }
private:
    pthread_spinlock_t m_mutex;
};

class CASLock{
public:
    typedef ScopedLockImpl<CASLock> Lock; 
    CASLock(){
        m_mutex.clear();
    }

    ~CASLock(){

    }

    void lock(){
        while(std::atomic_flag_test_and_set_explicit(&m_mutex,std::memory_order_acquire));
    }

    void unlock(){
        std::atomic_flag_clear_explicit(&m_mutex,std::memory_order_release);
    }


private:
    volatile std::atomic_flag m_mutex;
};

template<typename T>
class ReadScopedLockImpl{
public:
    ReadScopedLockImpl(T & mutex) : m_mutex(mutex){
        m_mutex.rdlock();
        m_locked = true;
    }

    ~ReadScopedLockImpl(){
        m_mutex.unlock();
    }

    void lock(){
        if(!m_locked){
            m_mutex.rdlock();
            m_locked = true;
        }
    }

    void unlock(){
        if(m_locked){
            m_mutex.unlock();
            m_locked = false;
        }
    }
private:
    T & m_mutex;
    bool m_locked;
};

template<typename T>
class WriteScopedLockImpl{
public:
    WriteScopedLockImpl(T & mutex) : m_mutex(mutex){
        m_mutex.wrlock();
        m_locked = true;
    }

    ~WriteScopedLockImpl(){
        m_mutex.unlock();
    }

    void lock(){
        if(!m_locked){
            m_mutex.wrlock();
            m_locked = true;
        }
    }

    void unlock(){
        if(m_locked){
            m_mutex.unlock();
            m_locked = false;
        }
    }
private:
    T & m_mutex;
    bool m_locked;
};

class RWMutex{
public:

    typedef ReadScopedLockImpl<RWMutex> ReadLock;
    typedef WriteScopedLockImpl<RWMutex> WriteLock;

    RWMutex(){
        pthread_rwlock_init(&m_lock,nullptr);
    }


    ~RWMutex(){
        pthread_rwlock_destroy(&m_lock);
    }

    void rdlock(){
        pthread_rwlock_rdlock(&m_lock);
    }

    void wrlock(){
        pthread_rwlock_wrlock(&m_lock);
    }

    void unlock(){
        pthread_rwlock_unlock(&m_lock);
    }
private:
    pthread_rwlock_t m_lock;
};

//! 测试使用
class NullRWMutex{
public:

    typedef ReadScopedLockImpl<NullRWMutex> ReadLock;
    typedef WriteScopedLockImpl<NullRWMutex> WriteLock;

    void rdlock(){
        
    }

    void wrlock(){
        
    }

    void unlock(){
       
    }
};

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