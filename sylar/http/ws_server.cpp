/*
 * @Descripttion: 
 * @version: 
 * @Author: zsj
 * @Date: 2020-06-29 10:14:28
 * @LastEditors: zsj
 * @LastEditTime: 2020-07-05 14:39:31
 */ 
#include"ws_server.h"
#include"sylar/log.h"


namespace sylar
{
namespace http
{
    
static Logger::ptr g_logger = SYLAR_LOG_NAME("system");


WSServer::WSServer(IOManager * worker
            ,IOManager * accept_worker)
    :TcpServer(worker,accept_worker){
    m_dispatch.reset(new WSServletDispatch);
    m_type = "websocket_server";
}

void WSServer::handleClient(Socket::ptr client){
    SYLAR_LOG_DEBUG(g_logger) << "handleClient " << *client;
    WSSession::ptr session(new WSSession(client));
    do{
        HttpRequest::ptr  header = session->handleShake();
        if(!header){
            SYLAR_LOG_ERROR(g_logger) << "handleShake error";
            break;
        }

        WSServlet::ptr servlet = m_dispatch->getWSServlet(header->getPath());
        if(!servlet){
            SYLAR_LOG_ERROR(g_logger) << "no match WSServlet";
            break;
        }
        int rt = servlet->onConnect(header,session);
        if(rt){
            SYLAR_LOG_ERROR(g_logger) << "onConnect return "<<rt;
            break;
        }

        while(true){
            auto msg = session->recvMessage();
            if(!msg){
                break;
            }

            rt = servlet->handle(header,msg,session);
            if(rt){
                SYLAR_LOG_ERROR(g_logger) << "handler return "<<rt;
                break;
            }
        }

        servlet->onClose(header,session);
    }while(0);

    session->close();
}


} // namespace http

} // namespace sylar

