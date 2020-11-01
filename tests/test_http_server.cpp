/*
 * @Descripttion:
 * @version:
 * @Author: zsj
 * @Date: 2020-06-16 17:57:49
 * @LastEditors: zsj
 * @LastEditTime: 2020-06-28 22:55:45
 */
#include "../sylar/http/http_server.h"
#include "../sylar/iomanager.h"
#include "../sylar/log.h"

static sylar::Logger::ptr g_logger = SYLAR_LOG_ROOT();

void run() {
    sylar::http::HttpServer::ptr server(new sylar::http::HttpServer);
    sylar::Address::ptr addr =
        sylar::Address::LookupAnyIPAddress("0.0.0.0:8020");
    while (!server->bind(addr, false)) {
        sleep(2);
    }

    auto sd = server->getServletDispatch();
    sd->addServlet("/sylar/xx",
                   [](sylar::http::HttpRequest::ptr req,
                      sylar::http::HttpResponse::ptr rsp,
                      sylar::http::HttpSession::ptr session) {
                       rsp->setBody(req->toString());
                       return 0;
                   });

    sd->addGlobalServlet("/sylar/*",
                         [](sylar::http::HttpRequest::ptr req,
                            sylar::http::HttpResponse::ptr rsp,
                            sylar::http::HttpSession::ptr session) {
                             std::string body = "Global:\r\n" + req->toString();
                             rsp->setBody(body);
                             return 0;
                         });

    server->start();
}

int main(int agrc, char** argv) {
    sylar::IOManager iom(2);
    iom.schedule(run);
}