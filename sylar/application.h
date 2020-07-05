/*
 * @Descripttion: 
 * @version: 
 * @Author: zsj
 * @Date: 2020-06-22 21:39:14
 * @LastEditors: zsj
 * @LastEditTime: 2020-07-05 14:45:17
 */ 
#pragma once

#include"http/http_server.h"

namespace sylar
{

class Application{

public:
    Application();


    static Application * GetInstance(){return s_instance;}

    bool init(int argc,char ** argv);

    bool run();

    bool getServer(const std::string & type,std::vector<TcpServer::ptr> & svrs);
private:
    

    int main(int argc,char ** argv);
    int run_fiber();
private:
    int m_argc = 0;
    char ** m_argv = nullptr;

    // std::vector<http::HttpServer::ptr> m_httpservers;
    std::map<std::string,std::vector<TcpServer::ptr>> m_servers;
    IOManager::ptr m_mainIOManager;
    static Application* s_instance;

};

} // namespace sylar
