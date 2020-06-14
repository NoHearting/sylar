/*
 * @Descripttion: 
 * @version: 
 * @Author: zsj
 * @Date: 2020-06-13 18:32:18
 * @LastEditors: zsj
 * @LastEditTime: 2020-06-14 10:02:20
 */ 
#include"../sylar/hook.h"
#include"../sylar/iomanager.h"
#include"../sylar/log.h"
#include"../sylar/util.h"

static sylar::Logger::ptr g_logger = SYLAR_LOG_ROOT();

void test_sleep(){
    sylar::IOManager iom;
    iom.schedule([](){
        sleep(2);
        SYLAR_LOG_INFO(g_logger) << "sleep 2";
    });

    iom.schedule([](){
        sleep(3);
        SYLAR_LOG_INFO(g_logger) << "sleep 3";
    });

    SYLAR_LOG_INFO(g_logger) << "test_sleep";
}


int main(int argc,char ** argv){
    test_sleep();
}


