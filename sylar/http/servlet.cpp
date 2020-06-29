/*
 * @Descripttion: 
 * @version: 
 * @Author: zsj
 * @Date: 2020-06-16 21:04:36
 * @LastEditors: zsj
 * @LastEditTime: 2020-06-29 20:14:36
 */ 
#include"servlet.h"
#include<fnmatch.h>


namespace sylar
{

namespace http
{
int32_t FunctionServlet::handle(sylar::http::HttpRequest::ptr request
        ,sylar::http::HttpResponse::ptr response
        ,sylar::http::HttpSession::ptr session){
    return m_cb(request,response,session);
}


ServletDispatch::ServletDispatch()
        :Servlet("ServletDispatch") {
        m_default.reset(new NotFoundServlet("sylar/1.0"));
}


int32_t ServletDispatch::handle(sylar::http::HttpRequest::ptr request
        ,sylar::http::HttpResponse::ptr response
        ,sylar::http::HttpSession::ptr session){
    auto slt = getMatchedServlet(request->getPath());
    if(slt){
        slt->handle(request,response,session);
    }
    return 0;
}

void ServletDispatch::addServlet(const std::string & uri,Servlet::ptr slt){
    RWMutexType::WriteLock lock(m_mutex);
    m_datas[uri] = slt;
}
void ServletDispatch::addServlet(const std::string & uri,FunctionServlet::callback cb){
    RWMutexType::WriteLock lock(m_mutex);
    m_datas[uri].reset(new FunctionServlet(cb));
}
void ServletDispatch::addGlobalServlet(const std::string & uri,Servlet::ptr slt){
    RWMutexType::WriteLock lock(m_mutex);
    for(auto it = m_globs.begin();it!=m_globs.end();++it){
        if(it->first == uri){
            m_globs.erase(it);
            break;
        }
    }
    m_globs.push_back(std::make_pair(uri,slt));
}
void ServletDispatch::addGlobalServlet(const std::string & uri,FunctionServlet::callback cb){
    return addGlobalServlet(uri,FunctionServlet::ptr(new FunctionServlet(cb)));
}


void ServletDispatch::delServlet(const std::string & uri){
    RWMutexType::WriteLock lock(m_mutex);
    m_datas.erase(uri);
}
void ServletDispatch::delGlobalServlet(const std::string & uri){
    RWMutexType::WriteLock lock(m_mutex);
    for(auto it = m_globs.begin();it!=m_globs.end();++it){
        if(it->first == uri){
            m_globs.erase(it);
            break;
        }
    }
}

Servlet::ptr ServletDispatch::getServlet(const std::string & uri){
    RWMutexType::ReadLock lock(m_mutex);
    auto it = m_datas.find(uri);
    return it == m_datas.end() ? nullptr : it->second;
}
Servlet::ptr ServletDispatch::getGlobalServlet(const std::string & uri){
    RWMutexType::ReadLock lock(m_mutex);
    for(auto it = m_globs.begin(); it != m_globs.end();++it){
        if(it->first == uri){
            return it->second;
        }
    }
    return nullptr;
}

Servlet::ptr ServletDispatch::getMatchedServlet(const std::string & uri){
    RWMutexType::ReadLock lock(m_mutex);
    auto mit = m_datas.find(uri);
    if(mit != m_datas.end()){
        return mit->second;
    }
    for(auto it = m_globs.begin(); it != m_globs.end();++it){
        if(!fnmatch(it->first.c_str(),uri.c_str(),0)){
            return it->second;
        }
    }
    return m_default;
}

NotFoundServlet::NotFoundServlet(const std::string & name)
    :Servlet("NotFoundServlet")
    ,m_name(name){
    
    m_content = "<html><head><title>404 NotFound </title></head>"
                "<body><center><h1>404 Not Found</h1></center>" 
                "<hr><center>" + name + "</center></body></html>";
    

}


int32_t NotFoundServlet::handle(sylar::http::HttpRequest::ptr request
        ,sylar::http::HttpResponse::ptr response
        ,sylar::http::HttpSession::ptr session){
    // static const std::string & RSP_BODY = "<!doctype html><html lang='en'><head><title>HTTP Status 404 – Not Found</title><style type='text/css'>h1 {font-family:Tahoma,Arial,sans-serif;color:white;background-color:#525D76;font-size:22px;} h2 {font-family:Tahoma,Arial,sans-serif;color:white;background-color:#525D76;font-size:16px;} h3 {font-family:Tahoma,Arial,sans-serif;color:white;background-color:#525D76;font-size:14px;} body {font-family:Tahoma,Arial,sans-serif;color:black;background-color:white;} b {font-family:Tahoma,Arial,sans-serif;color:white;background-color:#525D76;} p {font-family:Tahoma,Arial,sans-serif;background:white;color:black;font-size:12px;} a {color:black;} a.name {color:black;} .line {height:1px;background-color:#525D76;border:none;}</style></head><body><h1>HTTP Status 404 – Not Found</h1><hr class='line' /><p><b>Type</b> Status Report</p><p><b>Description</b> The origin server did not find a current representation for the target resource or is not willing to disclose that one exists.</p><hr class='line'/><h3>Apache Tomcat/9.0.1</h3></body></html>";
    response->setStatus(sylar::http::HttpStatus::NOT_FOUND);
    response->setHeader("Server","sylar/1.0.0");
    response->setHeader("Content-Type","text/html");
    response->setBody(m_content);
    return 0;
}

} // namespace http


} // namespace sylar
