/*
 * @Descripttion: 
 * @version: 
 * @Author: zsj
 * @Date: 2020-06-10 10:15:21
 * @LastEditors: zsj
 * @LastEditTime: 2020-06-10 11:23:05
 */ 
#include"../sylar/sylar.h"

static sylar::Logger::ptr g_logger = SYLAR_LOG_NAME("system");

void run_in_fiber(){
    SYLAR_LOG_INFO(g_logger) << "run in fiber begin";

    sylar::Fiber::YieldToHold();
    SYLAR_LOG_INFO(g_logger) << "run in fiber end";
    sylar::Fiber::YieldToHold();
}


int main(int argc,char ** argv){
    sylar::Thread::SetName("main");
    SYLAR_LOG_INFO(g_logger) << "main begin -1";
    {
        sylar::Fiber::GetThis();
        SYLAR_LOG_INFO(g_logger) << "main begin";
        sylar::Fiber::ptr fiber(new sylar::Fiber(run_in_fiber));
        fiber->swapIn();
        SYLAR_LOG_INFO(g_logger) << "main after swapIn";
        fiber->swapIn();
        SYLAR_LOG_INFO(g_logger) << "main after end";
        fiber->swapIn();
    }
    SYLAR_LOG_INFO(g_logger) << "main end 2";
}