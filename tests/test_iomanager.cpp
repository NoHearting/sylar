/*
 * @Descripttion: 
 * @version: 
 * @Author: zsj
 * @Date: 2020-06-10 23:04:33
 * @LastEditors: zsj
 * @LastEditTime: 2020-06-12 22:41:36
 */ 
#include"../sylar/sylar.h"
#include"../sylar/iomanager.h"
#include<sys/socket.h>
#include<sys/types.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<stdlib.h>
#include<stdio.h>
#include<fcntl.h>
#include<cstring>


static sylar::Logger::ptr g_logger = SYLAR_LOG_ROOT();

void test_fiber(){
    SYLAR_LOG_INFO(g_logger) << "test fiber";

    int sock = socket(PF_INET,SOCK_STREAM,0);
    SYLAR_ASSERT(sock != -1);
    fcntl(sock,F_SETFL,O_NONBLOCK);  //异步io

    struct sockaddr_in serv_addr;
    bzero(&serv_addr,sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(80);
    serv_addr.sin_addr.s_addr = inet_addr("39.156.69.79");

   

    int rt = connect(sock,(struct sockaddr*)&serv_addr,sizeof(serv_addr));
    if(rt == -1){
        SYLAR_LOG_INFO(g_logger) << "connect failed error: "<<strerror(errno);
    }
    else{
        sylar::IOManager::GetThis()->addEvent(sock,sylar::IOManager::WRITE,[](){
            SYLAR_LOG_INFO(g_logger) << "connected";
        });
    }
    
}

void test1(){
    sylar::IOManager iom(2,false);
    iom.schedule(&test_fiber);

    
}

sylar::Timer::ptr s_timer;
void test_timer(){
    sylar::IOManager iom(2);
    s_timer = iom.addTimer(1000,[](){
        static int i = 0;
        SYLAR_LOG_INFO(g_logger) << " hello i = " << i;
        if(++i == 3){
            // SYLAR_LOG_INFO(g_logger) << "取消事件";
            // s_timer->cancel();
            s_timer->reset(2000,true);
        }
    },true);
    // iom.addTimer(5000,[](){
    //     SYLAR_LOG_INFO(g_logger) << " test!!";
    // },true);
    SYLAR_LOG_INFO(g_logger) << "addTimer end in test_timer";
}

int main(int arhc,char ** argv)
{
    // test1();
    test_timer();
    
}