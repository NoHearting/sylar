/*
 * @Descripttion: 
 * @version: 
 * @Author: zsj
 * @Date: 2020-06-21 21:01:21
 * @LastEditors: zsj
 * @LastEditTime: 2020-06-21 22:32:18
 */ 
#include"../sylar/http/http_server.h"
#include"../sylar/log.h"
#include"../sylar/iomanager.h"

static sylar::Logger::ptr g_logger = SYLAR_LOG_ROOT();

void run(){
    g_logger->setLevel(sylar::LogLevel::INFO);
    sylar::Address::ptr addr = sylar::Address::LookupAnyIPAddress("0.0.0.0:8020");
    if(!addr){
        SYLAR_LOG_ERROR(g_logger) << "get address error";
        return;
    }
    sylar::http::HttpServer::ptr http_server(new sylar::http::HttpServer(true));
    while(!http_server->bind(addr)){
        SYLAR_LOG_ERROR(g_logger) << "bind " << *addr<<" fail";
        sleep(1);
    }
    http_server->start();
}

int main(int argc, char const *argv[])
{
    sylar::IOManager iom(4);
    iom.schedule(run);
    return 0;
}


/**
 * Requests per second:    28580.34 [#/sec] (mean)   3 threads
 * Requests per second:    29272.45 [#/sec] (mean)   2 threads
 * Requests per second:    30526.03 [#/sec] (mean)   1 threads
 * 
 * Requests per second:    27146.24 [#/sec] (mean)   4 threads   ngnix
 */ 