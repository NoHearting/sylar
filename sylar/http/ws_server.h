/*
 * @Descripttion: 
 * @version: 
 * @Author: zsj
 * @Date: 2020-06-29 10:09:29
 * @LastEditors: zsj
 * @LastEditTime: 2020-06-29 10:35:49
 */ 
#pragma once

#include "sylar/tcp_server.h"
#include "ws_session.h"
#include "ws_servlet.h"

namespace sylar
{

namespace http
{

class WSServer : public TcpServer{
public:
    typedef std::shared_ptr<WSServer> ptr;

    WSServer(IOManager * worker = IOManager::GetThis()
            ,IOManager * accept_worker = IOManager::GetThis());
    WSServletDispatch::ptr getWSServletDispatch()const {return m_dispatch;}

    void setWSServletDispatch(WSServletDispatch::ptr v){m_dispatch = v;}

protected:
    virtual void handleClient(Socket::ptr client) override;
private:
    WSServletDispatch::ptr m_dispatch;
};



} // namespace http
    
} // namespace sylar
