/*
 * @Descripttion: 
 * @version: 
 * @Author: zsj
 * @Date: 2020-06-17 22:12:56
 * @LastEditors: zsj
 * @LastEditTime: 2020-06-21 16:12:44
 */ 
#include"../sylar/http/http_connection.h"
#include<iostream>
#include"../sylar/log.h"
#include"../sylar/iomanager.h"

static sylar::Logger::ptr g_logger = SYLAR_LOG_ROOT();

void test_pool(){
    sylar::http::HttpConnectionPool::ptr pool(new sylar::http::HttpConnectionPool(
        "www.sylar.top","",80,10,1000*30,5));
    sylar::IOManager::GetThis()->addTimer(1000,[pool](){
        auto r = pool->doGet("/",300);
        SYLAR_LOG_INFO(g_logger) << r->toString();
    },true);

}


void run(){
    sylar::Address::ptr addr = sylar::Address::LookupAnyIPAddress("www.xl-zsj.top:80");
    if(!addr){
        SYLAR_LOG_INFO(g_logger)<<"get addr error";
        return ;
    }

    sylar::Socket::ptr sock = sylar::Socket::CreateTCP(addr);
    bool rt = sock->connect(addr);
    if(!rt){
        SYLAR_LOG_INFO(g_logger) << "connect "<<*addr<<" failed";
        return;
    }
    sylar::http::HttpConnection::ptr conn(new sylar::http::HttpConnection(sock));
    sylar::http::HttpRequest::ptr req(new sylar::http::HttpRequest);
    req->setHeader("Host","www.baidu.com");
    SYLAR_LOG_INFO(g_logger) << "req:"<<std::endl<<*req;

    conn->sendRequest(req);
    auto rsp = conn->recvResponse();
    if(!rsp){
        SYLAR_LOG_INFO(g_logger) << "recv response error";
        return;
    }

    // SYLAR_LOG_INFO(g_logger) << "rsp:"<<std::endl<<*rsp;


    SYLAR_LOG_INFO(g_logger) << "========================================";
    auto rt2 = sylar::http::HttpConnection::DoGet("http://www.baidu.com/",300);
    SYLAR_LOG_INFO(g_logger) << "result="<< rt2->result
            << " error="<<rt2->error
            << " rsp="<<(rt2->response ? rt2->response->toString() : "");
    test_pool();
}



int main(int argc,char ** argv)
{
    sylar::IOManager iom(2);
    iom.schedule(run);


}