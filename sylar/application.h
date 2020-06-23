/*
 * @Descripttion: 
 * @version: 
 * @Author: zsj
 * @Date: 2020-06-22 21:39:14
 * @LastEditors: zsj
 * @LastEditTime: 2020-06-23 10:49:36
 */ 
#pragma once

#include"http/http_server.h"

namespace sylar
{

class Application{

public:
    Application();

    bool init(int argc,char ** argv);

    bool run();
private:
    

    int main(int argc,char ** argv);
    int run_fiber();
private:
    int m_argc = 0;
    char ** m_argv = nullptr;

    std::vector<http::HttpServer::ptr> m_httpservers;
    static Application* s_instance;

};

} // namespace sylar
