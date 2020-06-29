/*
 * @Descripttion: 
 * @version: 
 * @Author: zsj
 * @Date: 2020-06-16 17:30:29
 * @LastEditors: zsj
 * @LastEditTime: 2020-06-29 20:15:29
 */ 
#pragma once

#include"../tcp_server.h"
#include"http_session.h"
#include"servlet.h"
#include<memory>

namespace sylar
{

namespace http
{

class HttpServer : public TcpServer{
public:
    typedef std::shared_ptr<HttpServer> ptr;

    HttpServer(bool keepalive = false
                ,sylar::IOManager * worker = sylar::IOManager::GetThis()
                ,sylar::IOManager * accept_worker = sylar::IOManager::GetThis());


    ServletDispatch::ptr getDispatch()const{return m_dispatch;}
    void setServletDispatch(ServletDispatch::ptr dispatch){m_dispatch = dispatch;}

    virtual void setName(const std::string & v)override;
protected:
    virtual void handleClient(Socket::ptr client)override;

private:
    bool m_isKeepalive;
    ServletDispatch::ptr m_dispatch;
};


} // namespace http


} // namespace sylar
