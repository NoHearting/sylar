/*
 * @Descripttion: 
 * @version: 
 * @Author: zsj
 * @Date: 2020-06-16 10:35:16
 * @LastEditors: zsj
 * @LastEditTime: 2020-06-29 19:18:16
 */ 
#include"tcp_server.h"
#include"config.h"
#include"log.h"

static sylar::ConfigVar<uint64_t>::ptr g_tcp_server_read_timeout = 
    sylar::Config::Lookup("tcp_server.read_timeout",(uint64_t)(60 * 1000 * 2),
    "tcp server read timeout");

static sylar::Logger::ptr g_logger = SYLAR_LOG_NAME("system");

namespace sylar
{
TcpServer::TcpServer(sylar::IOManager * worker,sylar::IOManager * accept_worker)
    :m_worker(worker)
    ,m_acceptWorker(accept_worker)
    ,m_recvTimeout(g_tcp_server_read_timeout->getValue())
    ,m_name("sylar/1.0.0")
    ,m_isStop(true){

}

TcpServer::~TcpServer(){
    for(auto & i : m_socks){
        i->close();
    }
    m_socks.clear();
}

bool TcpServer::bind(sylar::Address::ptr addr,bool ssl){
    std::vector<Address::ptr> addrs;
    std::vector<Address::ptr> failed;
    addrs.push_back(addr);
    return bind(addrs,failed,ssl);
}
bool TcpServer::bind(const std::vector<Address::ptr> & addrs
    ,std::vector<Address::ptr> & failed,bool ssl){
    for(auto & addr : addrs){
        // Socket::ptr sock = Socket::CreateTCP(addr);
        Socket::ptr sock = ssl ? SSLSocket::CreateTCP(addr) : Socket::CreateTCP(addr);
        if(!sock->bind(addr)){
            SYLAR_LOG_ERROR(g_logger) << "bind fail errno="
                << errno <<" errstr="<<strerror(errno)
                << " addr=[" << addr->toString() << "]";
            failed.push_back(addr);
            continue;

        }
        if(!sock->listen()){
            SYLAR_LOG_ERROR(g_logger) << "listen fail errno="
                << errno << " errstr="<<strerror(errno)
                <<" addr=[" << addr->toString() <<"]";
            failed.push_back(addr);
            continue;
        }
        m_socks.push_back(sock);
    }
    if(!failed.empty()){
        m_socks.clear();
        return false;
    }

    for(auto & i : m_socks){
        SYLAR_LOG_INFO(g_logger) << "server bind success: " << *i;
    }

    return true;
}

void TcpServer::startAccept(Socket::ptr sock){
    while(!m_isStop){
        Socket::ptr client = sock->accept();
        if(client){
            client->setRecvTimeout(m_recvTimeout);
            m_worker->schedule(std::bind(&TcpServer::handleClient,shared_from_this(),client));
        }
        else{
            SYLAR_LOG_ERROR(g_logger) << "accept errno="<<errno
                <<" errstr="<<strerror(errno);
        }
    }
}

bool TcpServer::start(){
    if(!m_isStop){
        return true;
    }
    m_isStop = false;
    for(auto & sock : m_socks){
        m_acceptWorker->schedule(std::bind(&TcpServer::startAccept,
                            shared_from_this(),sock));
    }
    return true;
}
void TcpServer::stop(){
    m_isStop = true;
    auto self = shared_from_this();
    m_acceptWorker->schedule([this,self](){
        for(auto & sock : m_socks){
            sock->cancelAll();
            sock->close();
        }
        m_socks.clear();
    });
}
void TcpServer::handleClient(Socket::ptr client){
    SYLAR_LOG_INFO(g_logger) << "handleClient: " << *client;
}


bool TcpServer::loadCertificates(const std::string & cert_file
    ,const std::string & key_file){
   for(auto & i : m_socks){
       auto ssl_socket = std::dynamic_pointer_cast<SSLSocket>(i);
       if(ssl_socket){
           if(!ssl_socket->loadCertificates(cert_file,key_file)){
               return false;
           }
       }
   } 
   return true;
}
} // namespace sylar
