/*
 * @Descripttion: 
 * @version: 
 * @Author: zsj
 * @Date: 2020-06-16 16:48:30
 * @LastEditors: zsj
 * @LastEditTime: 2020-06-16 22:08:14
 */ 
#pragma once
#include"../socket_stream.h"
#include"http.h"

namespace sylar
{

namespace http
{

class HttpSession : public SocketStream{
public:
    typedef std::shared_ptr<HttpSession> ptr;

    HttpSession(Socket::ptr sock,bool owner = true);


    HttpRequest::ptr recvRequest();
    int sendResponse(HttpResponse::ptr rsp);

};

    
} // namespace http


} // namespace sylar
