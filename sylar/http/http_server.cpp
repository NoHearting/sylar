/*
 * @Descripttion: 
 * @version: 
 * @Author: zsj
 * @Date: 2020-06-16 17:39:20
 * @LastEditors: zsj
 * @LastEditTime: 2020-07-05 14:39:54
 */ 
#include"http_server.h"
#include"../log.h"

static sylar::Logger::ptr g_logger = SYLAR_LOG_NAME("system");

namespace sylar
{

namespace http
{
    
HttpServer::HttpServer(bool keepalive
                ,sylar::IOManager * worker
                ,sylar::IOManager * accept_worker)
    :TcpServer(worker,accept_worker)
    ,m_isKeepalive(keepalive){
    m_dispatch.reset(new ServletDispatch);
    m_type = "http";
}

void HttpServer::setName(const std::string & v){
    TcpServer::setName(v);
    m_dispatch->setDefaultServlet(std::make_shared<NotFoundServlet>(v));
}

void HttpServer::handleClient(Socket::ptr client){
    HttpSession::ptr session(new HttpSession(client));
    do{
        auto req = session->recvRequest();
        if(!req){
            SYLAR_LOG_WARN(g_logger) << "recv http request fail, errno="
                << errno << " errstr=" << strerror(errno)
                << " cliet:"<<*client;
                break;
        }
        HttpResponse::ptr rsp(new HttpResponse(req->getVersion(),req->isClose() || !m_isKeepalive));
        
        m_dispatch->handle(req,rsp,session);
        // rsp->setBody("hello sylar");

        // SYLAR_LOG_ERROR(g_logger) << "request:" << std::endl
        //     << *req;
        // SYLAR_LOG_ERROR(g_logger) << "response:" << std::endl
        //     << *rsp;
        session->sendResponse(rsp);
    }while(m_isKeepalive);
    session->close();
}
} // namespace http


} // namespace sylar

