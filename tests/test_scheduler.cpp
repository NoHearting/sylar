/*
 * @Descripttion: 
 * @version: 
 * @Author: zsj
 * @Date: 2020-06-10 15:03:03
 * @LastEditors: zsj
 * @LastEditTime: 2020-07-31 14:46:46
 */ 
// #include"../sylar/sylar.h"
#include"sylar/log.h"
#include"sylar/scheduler.h"
#include<unistd.h>


static sylar::Logger::ptr g_logger = SYLAR_LOG_ROOT();

void test_fiber(){
    SYLAR_LOG_INFO(g_logger) << "test in fiber";

    //钩子函数   想要测试此用例  必须关闭sleep的钩子
    sleep(1);
    static int s_count = 5;
    if (--s_count > 0)
    {
        SYLAR_LOG_DEBUG(g_logger) << "task add schedule";
        sylar::Scheduler::GetThis()->schedule(&test_fiber,sylar::GetThreadId());
    }
    
    
}


int main(int argc,char * argv[])
{
    SYLAR_LOG_INFO(g_logger) << "main";
    sylar::Scheduler sc(1,false,"test");
    
    sc.start();
    SYLAR_LOG_INFO(g_logger) << "schedule";
    sc.schedule(&test_fiber);
    sc.stop();
    SYLAR_LOG_INFO(g_logger) << "over";
}