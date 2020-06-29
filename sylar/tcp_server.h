/*
 * @Descripttion: 
 * @version: 
 * @Author: zsj
 * @Date: 2020-06-16 10:26:38
 * @LastEditors: zsj
 * @LastEditTime: 2020-06-29 19:14:28
 */ 
#pragma once
#include<memory>
#include<functional>
#include<vector>
#include"iomanager.h"
#include"socket.h"
#include"address.h"
#include"noncopyable.h"



namespace sylar{

class TcpServer : public std::enable_shared_from_this<TcpServer>
                , public Noncopyable{

public:
    typedef std::shared_ptr<TcpServer> ptr;

    TcpServer(sylar::IOManager * worker = sylar::IOManager::GetThis()
             ,sylar::IOManager * accept_worker = sylar::IOManager::GetThis());
    virtual ~TcpServer();

    virtual bool bind(sylar::Address::ptr addr,bool ssl);
    virtual bool bind(const std::vector<Address::ptr> & addrs
        ,std::vector<Address::ptr> & failed,bool ssl);

    
    bool loadCertificates(const std::string & cert_file,const std::string & key_file);

    virtual bool start();
    virtual void stop();

    uint64_t getRecvTimeout()const{return m_recvTimeout;}
    std::string getName()const{return m_name;}
    void setRecvTimeout(uint64_t v){m_recvTimeout = v;}
    virtual void setName(const std::string & v){m_name = v;}

    bool isStop()const{return m_isStop;}

protected:
    virtual void handleClient(Socket::ptr client);
    virtual void startAccept(Socket::ptr sock);
private:
    std::vector<Socket::ptr> m_socks;
    IOManager * m_worker;
    IOManager * m_acceptWorker;
    uint64_t m_recvTimeout;
    std::string m_name;
    bool m_isStop;
};


}