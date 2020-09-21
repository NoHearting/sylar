/*
 * @Descripttion: 
 * @version: 
 * @Author: zsj
 * @Date: 2020-06-08 11:18:54
 * @LastEditors: zsj
 * @LastEditTime: 2020-07-05 20:53:45
 */ 

#include "../sylar/sylar.h"
#include<time.h>
#include<iostream>

// static sylar::Logger::ptr g_logger = SYLAR_LOG_ROOT();

int count = 0;
// sylar::RWMutex s_mutex;
sylar::Mutex s_mutex;
void func1(){
    
    SYLAR_LOG_INFO(g_logger) << "name: "<<sylar::Thread::GetName()
                        << "\tthis.name: "<<sylar::Thread::GetThis()->getName()
                        << "\tid: "<<sylar::GetThreadId()
                        << "\tthis.id: "<<sylar::Thread::GetThis()->getId();
    for(int i = 0;i<1000000;i++){
        // sylar::RWMutex::WriteLock lock(s_mutex);
        sylar::Mutex::Lock lock(s_mutex);
        count++;
    }
   
    SYLAR_LOG_INFO(g_logger) << "over for : " << sylar::Thread::GetName();

}

void func2(){
    while(true){
        SYLAR_LOG_INFO(g_logger) << "xxxxxxxxxxxxxxxxxxxxxxxxxx";
    }
}

void func3(){
    while(true){
        SYLAR_LOG_INFO(g_logger) << "==========================";
    }
}

int main(int argc,char * argv[])
{
    SYLAR_LOG_INFO(g_logger) << "thread test begin";
    YAML::Node root = YAML::LoadFile("/home/zsj/workspace/sylar/bin/conf/log2.yml");
    sylar::Config::LoadFromYaml(root);
    std::vector<sylar::Thread::ptr> thrs;
    for(int i = 0;i<2;i++){
        sylar::Thread::ptr thr(new sylar::Thread(&func2,"name_"+std::to_string(i*2)));
        sylar::Thread::ptr thr2(new sylar::Thread(&func3,"name_"+std::to_string(i*2+1)));
        thrs.push_back(thr);
        thrs.push_back(thr2);

    }

    for(int i = 0;thrs.size();++i){
        thrs[i]->join();

    }
    SYLAR_LOG_INFO(g_logger) << "thread test end";
    SYLAR_LOG_INFO(g_logger) << count;

    
}