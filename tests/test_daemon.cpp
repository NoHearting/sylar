/*
 * @Descripttion: 
 * @version: 
 * @Author: zsj
 * @Date: 2020-06-21 20:19:26
 * @LastEditors: zsj
 * @LastEditTime: 2020-06-21 20:44:36
 */ 
#include"../sylar/daemon.h"
#include"../sylar/iomanager.h"
#include"../sylar/log.h"
#include"../sylar/timer.h"

static sylar::Logger::ptr g_logger = SYLAR_LOG_ROOT();

sylar::Timer::ptr timer;
int server_main(int argc,const char ** argv){
    SYLAR_LOG_INFO(g_logger) << sylar::ProcessInfoMgr::GetInstance()->toString();
    sylar::IOManager iom;
    
    timer = iom.addTimer(1000,[](){
        SYLAR_LOG_INFO(g_logger) << "onTimer";
        static int count = 0;
        if(++count > 10){
            timer->cancel();
        }
    },true);
    return 0;
}

int main(int argc, char const *argv[])
{
    return sylar::start_daemon(argc,argv,server_main,argc != 1);
}
