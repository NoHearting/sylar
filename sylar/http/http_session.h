/*
 * @Descripttion:
 * @version:
 * @Author: zsj
 * @Date: 2020-06-16 16:48:30
 * @LastEditors: zsj
 * @LastEditTime: 2020-10-12 15:43:36
 */
#pragma once
#include "http.h"
#include "sylar/streams/socket_stream.h"

namespace sylar {

namespace http {

class HttpSession : public SocketStream {
public:
    typedef std::shared_ptr<HttpSession> ptr;

    HttpSession(Socket::ptr sock, bool owner = true);


    HttpRequest::ptr recvRequest();
    int sendResponse(HttpResponse::ptr rsp);
};


}  // namespace http


}  // namespace sylar
