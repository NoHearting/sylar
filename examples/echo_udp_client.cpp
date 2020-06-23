/*
 * @Descripttion: 
 * @version: 
 * @Author: zsj
 * @Date: 2020-06-23 15:43:37
 * @LastEditors: zsj
 * @LastEditTime: 2020-06-23 16:15:04
 */ 
#include"sylar/socket.h"
#include"sylar/iomanager.h"
#include"sylar/log.h"
#include"sylar/thread.h"
#include<cstdlib>


static sylar::Logger::ptr g_logger = SYLAR_LOG_ROOT();

const char * ip = nullptr;
uint16_t port = 0;
static sylar::Mutex s_mutex;  //! 用于同步输出信息，方便观察
void run(){
    sylar::IPAddress::ptr addr = sylar::Address::LookupAnyIPAddress(ip);
    if(!addr){
        SYLAR_LOG_ERROR(g_logger) << "invalid ip:" << ip;
        return;
    }
    addr->setPort(port);
    sylar::Socket::ptr sock = sylar::Socket::CreateUDP(addr);
    sylar::IOManager::GetThis()->schedule([addr,sock](){
        SYLAR_LOG_INFO(g_logger) << "begin recv";
        while(true){
            char buff[1024];
            int len = sock->recvFrom(buff,1024,addr);
            if(len < 0){
                std::cout<<std::endl;
                SYLAR_LOG_INFO(g_logger) << "recv: " << std::string(buff,len)
                    <<" from: "<<*addr;
            }
            {
                sylar::Mutex::Lock lock(s_mutex);
                std::cout<<"recv> "<<buff<<std::endl;
            }
            
        }
    });
    sleep(1);
    while(true){
        std::string line;
        { //! 加锁仅为观察数据方便
            sylar::Mutex::Lock lock(s_mutex);
            std::cout<<"input> ";
            std::getline(std::cin,line);
        }
        
        
        if(!line.empty()){
            int len = sock->sendTo(line.c_str(),line.size(),addr);
            if(len < 0){
                int err = sock->getError();
                SYLAR_LOG_ERROR(g_logger) << "send error err="<<err
                    <<" errstr="<<strerror(err) << " len=" << len
                    <<" addr="<<*addr
                    <<" sock="<<*sock;
            }
            else{
                std::cout << "send " << line <<" len:"<<len << std::endl;
            }
        }
        sleep(1);  //! 为了查看测试数据更直观
    }
}


int main(int argc, char ** argv) {
    if(argc < 3){
        SYLAR_LOG_ERROR(g_logger) << "Usage: "<<argv[0]<< "<ip> <port>";
        return 0;
    }
    ip = argv[1];
    port = atoi(argv[2]);
    sylar::IOManager iom(2);
    iom.schedule(run);
    return 0;
}