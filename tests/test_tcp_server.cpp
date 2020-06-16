/*
 * @Descripttion: 
 * @version: 
 * @Author: zsj
 * @Date: 2020-06-16 11:16:16
 * @LastEditors: zsj
 * @LastEditTime: 2020-06-16 11:43:37
 */ 
#include"../sylar/tcp_server.h"
#include"../sylar/iomanager.h"
#include"../sylar/log.h"

static sylar::Logger::ptr g_logger = SYLAR_LOG_ROOT();

void run(){
    auto addr = sylar::Address::LookupAny("0.0.0.0:8035");
    // auto addr2 = sylar::UnixAddress::ptr(new sylar::UnixAddress("/tmp/unix_addr2"));
    // SYLAR_LOG_INFO(g_logger) << *addr <<" - "<<*addr2;
    std::vector<sylar::Address::ptr> addrs;
    addrs.push_back(addr);
    // addrs.push_back(addr2);
    sylar::TcpServer::ptr  tcp_server(new sylar::TcpServer);
    std::vector<sylar::Address::ptr> fails;
    while(!tcp_server->bind(addrs,fails)){
        sleep(2);
    }
    tcp_server->start();

}

int main(int argc,char ** argv)
{
    sylar::IOManager iom(2);
    iom.schedule(run);

}