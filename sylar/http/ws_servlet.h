/*
 * @Descripttion: 
 * @version: 
 * @Author: zsj
 * @Date: 2020-06-29 09:27:07
 * @LastEditors: zsj
 * @LastEditTime: 2020-06-29 10:18:13
 */ 
#pragma once

#include "ws_session.h"
#include "sylar/thread.h"
#include "servlet.h"

#include <functional>

namespace sylar
{

namespace http
{

class WSServlet : public Servlet{
public:

    typedef std::shared_ptr<WSServlet> ptr;

    WSServlet(const std::string & name)
        :Servlet(name){
    }

    virtual ~WSServlet(){}

     virtual int32_t handle(sylar::http::HttpRequest::ptr request
        ,sylar::http::HttpResponse::ptr response
        ,sylar::http::HttpSession::ptr session)override{
        return 0;
    }

    virtual int32_t onConnect(HttpRequest::ptr header
                             ,WSSession::ptr session) = 0;
    virtual int32_t onClose(HttpRequest::ptr header
                             ,WSSession::ptr session) = 0;
    virtual int32_t handle(HttpRequest::ptr header
                        , WSFrameMessage::ptr msg
                        , WSSession::ptr session) = 0;
    
    const std::string & getName()const {return m_name;}
protected:
    std::string m_name;

};


class FunctionWSServlet : public WSServlet{
public:
    typedef std::shared_ptr<FunctionWSServlet> ptr;
    typedef std::function<int32_t(HttpRequest::ptr 
                                 ,WSSession::ptr )> on_connect_cb;
    typedef std::function<int32_t(HttpRequest::ptr
                                 ,WSSession::ptr)> on_close_cb;
    typedef std::function<int32_t(HttpRequest::ptr
                                 ,WSFrameMessage::ptr
                                 ,WSSession::ptr)> callback;
    
    FunctionWSServlet(callback cb, on_connect_cb  connect_cb = nullptr
                    ,on_close_cb close_cb = nullptr);

    virtual int32_t onConnect(HttpRequest::ptr header
                             ,WSSession::ptr session)override;
    virtual int32_t onClose(HttpRequest::ptr header
                             ,WSSession::ptr session)override;
    virtual int32_t handle(HttpRequest::ptr header
                        , WSFrameMessage::ptr msg
                        , WSSession::ptr session)override;
                        
                        
protected:
    callback m_callback;
    on_connect_cb m_onConnect;
    on_close_cb m_onClose;;
};


class WSServletDispatch : public ServletDispatch{
public:
    typedef std::shared_ptr<WSServletDispatch> ptr;
    typedef RWMutex RWMutexType;

    WSServletDispatch();

    void addServlet(const std::string & uri
                    ,FunctionWSServlet::callback cb
                    ,FunctionWSServlet::on_connect_cb connect_cb = nullptr
                    ,FunctionWSServlet::on_close_cb close_cb = nullptr);
    void addGlobalServlet(const std::string & uri
                    ,FunctionWSServlet::callback cb
                    ,FunctionWSServlet::on_connect_cb connect_cb = nullptr
                    ,FunctionWSServlet::on_close_cb close_cb = nullptr);
    
    WSServlet::ptr getWSServlet(const std::string & uri);
};

} // namespace http

    
} // namespace sylar
