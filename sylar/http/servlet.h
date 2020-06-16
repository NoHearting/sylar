/*
 * @Descripttion: 
 * @version: 
 * @Author: zsj
 * @Date: 2020-06-16 20:43:13
 * @LastEditors: zsj
 * @LastEditTime: 2020-06-16 21:43:15
 */ 
#pragma once
#include<memory>
#include<functional>
#include<string>
#include<unordered_map>
#include"http.h"
#include"http_session.h"
#include"../thread.h"


namespace sylar
{

namespace http
{

class Servlet
{

public:

    typedef std::shared_ptr<Servlet> ptr;
    Servlet(/* args */) {}
    Servlet(const std::string & name):m_name(name){}
    virtual ~Servlet() {}

    virtual int32_t handle(sylar::http::HttpRequest::ptr request
        ,sylar::http::HttpResponse::ptr response
        ,sylar::http::HttpSession::ptr session) = 0;
    
    const std::string & getName()const{return m_name;}
protected:
    std::string m_name;

private:
    /* data */

};

class FunctionServlet : public Servlet
{

public:

    typedef std::shared_ptr<FunctionServlet> ptr;
    typedef std::function<int32_t(sylar::http::HttpRequest::ptr request
        ,sylar::http::HttpResponse::ptr response
        ,sylar::http::HttpSession::ptr session)>callback;
    
    FunctionServlet(callback cb):m_cb(cb){}
    FunctionServlet(/* args */):m_cb(nullptr) {}
    ~FunctionServlet() {}

    virtual int32_t handle(sylar::http::HttpRequest::ptr request
        ,sylar::http::HttpResponse::ptr response
        ,sylar::http::HttpSession::ptr session) override;
private:
    callback m_cb;
};


class NotFoundServlet;

class ServletDispatch : public Servlet
{

public:
    typedef std::shared_ptr<ServletDispatch>ptr;
    typedef RWMutex RWMutexType;
    ServletDispatch();
    ~ServletDispatch() {}

    virtual int32_t handle(sylar::http::HttpRequest::ptr request
        ,sylar::http::HttpResponse::ptr response
        ,sylar::http::HttpSession::ptr session) override;

    void addServlet(const std::string & uri,Servlet::ptr slt);
    void addServlet(const std::string & uri,FunctionServlet::callback cb);
    void addGlobalServlet(const std::string & uri,Servlet::ptr slt);
    void addGlobalServlet(const std::string & uri,FunctionServlet::callback cb);


    void delServlet(const std::string & uri);
    void delGlobalServlet(const std::string & uri);

    Servlet::ptr getDefaultServlet()const{return m_default;}
    void setDefaultServlet(Servlet::ptr v){
        RWMutexType::WriteLock lock(m_mutex);
        m_default = v;
    }

    Servlet::ptr getServlet(const std::string & uri);
    Servlet::ptr getGlobalServlet(const std::string & uri);

    Servlet::ptr getMatchedServlet(const std::string & uri);
private:
    /* data */
    std::unordered_map<std::string,Servlet::ptr> m_datas;

    std::vector<std::pair<std::string,Servlet::ptr>>m_globs;

    //默认servlet，所有路劲都没匹配时使用
    Servlet::ptr m_default;

    RWMutexType m_mutex;
};

    
class NotFoundServlet : public Servlet
{
private:
    /* data */
public:
    NotFoundServlet() :Servlet("NotFoundServlet"){}
    ~NotFoundServlet() {}


    int32_t handle(sylar::http::HttpRequest::ptr request
        ,sylar::http::HttpResponse::ptr response
        ,sylar::http::HttpSession::ptr session) override;
};

} // namespace http



} // namespace sylar
