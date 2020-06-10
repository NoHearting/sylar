/*
 * @Descripttion: 
 * @version: 
 * @Author: zsj
 * @Date: 2020-06-10 15:03:03
 * @LastEditors: zsj
 * @LastEditTime: 2020-06-10 18:40:55
 */ 
#include"../sylar/sylar.h"



static sylar::Logger::ptr g_logger = SYLAR_LOG_ROOT();

void test_fiber(){
    SYLAR_LOG_INFO(g_logger) << "test in fiber";
    sleep(1);
    static int s_count = 5;
    if (--s_count > 0)
    {
        sylar::Scheduler::GetThis()->schedule(&test_fiber,sylar::GetThreadId());
    }
    
    
}


int main(int argc,char * argv[])
{
    SYLAR_LOG_INFO(g_logger) << "main";
    sylar::Scheduler sc(3,false,"test");
    
    sc.start();
    SYLAR_LOG_INFO(g_logger) << "schedule";
    sc.schedule(&test_fiber);
    sc.stop();
    SYLAR_LOG_INFO(g_logger) << "over";
}