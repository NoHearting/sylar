/*
 * @Descripttion: 
 * @version: 
 * @Author: zsj
 * @Date: 2020-06-29 09:44:48
 * @LastEditors: zsj
 * @LastEditTime: 2020-06-29 10:00:02
 */ 
#include"ws_servlet.h"

namespace sylar
{
namespace http
{

FunctionWSServlet::FunctionWSServlet(callback cb, on_connect_cb connect_cb
                                ,on_close_cb close_cb)
    :WSServlet("FunctionWSServlet")
    ,m_callback(cb)
    ,m_onConnect(connect_cb)
    ,m_onClose(close_cb){
    
}

int32_t FunctionWSServlet::onConnect(HttpRequest::ptr header
                            ,WSSession::ptr session){
    if(m_onConnect){
        return m_onConnect(header,session);
    }
    return 0;
}
int32_t FunctionWSServlet::onClose(HttpRequest::ptr header
                            ,WSSession::ptr session){
    if(m_onClose){
        return m_onClose(header,session);
    }
    return 0;
}

int32_t FunctionWSServlet::handle(HttpRequest::ptr header
                    , WSFrameMessage::ptr msg
                    , WSSession::ptr session){
    if(m_callback){
        return m_callback(header,msg,session);
    }
    return 0;
}


WSServletDispatch::WSServletDispatch(){
    m_name = "WSServletDispatch";
}


void WSServletDispatch::addServlet(const std::string & uri
                    ,FunctionWSServlet::callback cb
                    ,FunctionWSServlet::on_connect_cb connect_cb
                    ,FunctionWSServlet::on_close_cb close_cb){
    ServletDispatch::addServlet(uri,std::make_shared<FunctionWSServlet>(cb,connect_cb,close_cb));
}
void WSServletDispatch::addGlobalServlet(const std::string & uri
                ,FunctionWSServlet::callback cb
                ,FunctionWSServlet::on_connect_cb connect_cb
                ,FunctionWSServlet::on_close_cb close_cb){
    ServletDispatch::addGlobalServlet(uri,std::make_shared<FunctionWSServlet>(cb,connect_cb,close_cb));
}

WSServlet::ptr WSServletDispatch::getWSServlet(const std::string & uri){
    auto slt = getMatchedServlet(uri);
    return std::dynamic_pointer_cast<WSServlet>(slt);
}


} // namespace http

} // namespace sylar
