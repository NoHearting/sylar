/*
 * @Descripttion: 
 * @version: 
 * @Author: zsj
 * @Date: 2020-06-16 14:35:46
 * @LastEditors: zsj
 * @LastEditTime: 2020-06-28 22:56:32
 */ 
#include"../sylar/tcp_server.h"
#include"../sylar/log.h"
#include"../sylar/iomanager.h"
#include"../sylar/socket.h"
#include"../sylar/bytearray.h"
#include"../sylar/address.h"
#include<vector>

static sylar::Logger::ptr g_logger = SYLAR_LOG_ROOT();

class EchoServer : public sylar::TcpServer{
public:
    EchoServer(int type);
    void handleClient(sylar::Socket::ptr client);

private:
    int m_type = 0;
};

EchoServer::EchoServer(int type)
    :m_type(type){

}
void EchoServer::handleClient(sylar::Socket::ptr client){
    SYLAR_LOG_INFO(g_logger) << "handleClient " << *client;
    sylar::ByteArray::ptr ba(new sylar::ByteArray);
    int t = 100;
    while(true){
        ba->clear();
        std::vector<iovec> iovs;
        ba->getWriteBuffers(iovs,1024);
        if(t-- < 0){
            break;
        }
        int rt = client->recv(&iovs[0],iovs.size());
        if(rt == 0){
            SYLAR_LOG_INFO(g_logger) << "client close:"<<*client;
            break;
        }
        else if(rt < 0){
            SYLAR_LOG_INFO(g_logger) << "client error rt="<<rt
                <<" errno"<<errno<<" errstr="<<strerror(errno);
            break;
        }
        ba->setPosition(ba->getPosition() + rt);
        ba->setPosition(0);
        if(m_type == 1){  //文本
            // SYLAR_LOG_INFO(g_logger) << ba->toString();
            std::cout<<ba->toString();
        }
        else{  //二进制
            // SYLAR_LOG_INFO(g_logger) << ba->toHexString();
            std::cout<<ba->toHexString();
        }
    }
}

int type = 1;

void run(){
    EchoServer::ptr es(new EchoServer(type));
    auto addr = sylar::Address::LookupAny("0.0.0.0:8020");
    while(!es->bind(addr,false)){
        sleep(2);
    }
    es->start();
}

int main(int argc,char **argv)
{
    if(argc < 2){
        SYLAR_LOG_INFO(g_logger) <<"used as["<<argv[0]<<" -t] or ["<<argv[0]<<" -b]";
        return 0;
    }
    if(strcmp(argv[1],"-b") == 0){
        type = 2;
    }
    sylar::IOManager iom(2);
    iom.schedule(run);




}