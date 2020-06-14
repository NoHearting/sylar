/*
 * @Descripttion: 
 * @version: 
 * @Author: zsj
 * @Date: 2020-06-13 16:32:43
 * @LastEditors: zsj
 * @LastEditTime: 2020-06-14 12:29:06
 */ 
#pragma once
#include<memory>
#include<string>
#include<sstream>
#include<sys/socket.h>
#include<sys/types.h>
#include<cstring>
#include<fstream>
#include<arpa/inet.h>
#include<unistd.h>
#include<stddef.h>
#include<sys/un.h>
#include<vector>
#include<map>

namespace sylar{

class IPAddress;
class Address{

public:
    typedef std::shared_ptr<Address> ptr;


    static Address::ptr LookupAny(const std::string & host,int family,int type,int protocol);
    static std::shared_ptr<IPAddress> LookupAnyIPAddress(const std::string & host,int family,int type,int protocol);
    static bool Lookup(std::vector<Address::ptr> & result,const std::string &host
        ,int family = AF_UNSPEC,int type = 0,int protocol = 0);
    static Address::ptr Create(const sockaddr * addr,socklen_t addrlen);

    static bool GetInterfaceAddress(std::multimap<std::string
        ,std::pair<Address::ptr,uint32_t>> & result
        ,int family = AF_UNSPEC);
    
    static bool GetInterfaceAddress(std::vector<std::pair<Address::ptr,uint32_t>> & result
        ,const std::string & iface
        ,int family = AF_UNSPEC);

    virtual ~Address(){}


    int getFamily() const;

    virtual const sockaddr * getAddress()const = 0;
    virtual const socklen_t getAddressLen() const = 0;

    virtual std::ostream & insert(std::ostream & os) const = 0;

    std::string toString();

    bool operator<(const Address & rhs) const;
    bool operator==(const Address & rhs) const;
    bool operator!=(const Address & rhs) const;

};

class IPAddress : public Address {
public:
    typedef std::shared_ptr<IPAddress> ptr;

    static IPAddress::ptr Create(const char * address,uint32_t port = 0);

    virtual IPAddress::ptr broadcastAddress(uint32_t prefix_len) = 0;
    virtual IPAddress::ptr networkAddress(uint32_t prefix_len) = 0;
    virtual IPAddress::ptr subNetmask(uint32_t prefix_len) = 0;

    virtual uint32_t getPort() const = 0;
    virtual void setPort(uint32_t v) = 0;
};

class IPv4Address : public IPAddress{
public:
    typedef std::shared_ptr<IPv4Address> ptr;


    static IPv4Address::ptr Create(const char * address,uint32_t port = 0);
    IPv4Address(sockaddr_in & addr);
    IPv4Address(uint32_t address = INADDR_ANY,uint32_t port = 0);
    ~IPv4Address(){}

    const sockaddr * getAddress()const override;
    const socklen_t getAddressLen() const override;
    std::ostream & insert(std::ostream & os) const override;
    IPAddress::ptr broadcastAddress(uint32_t prefix_len) override;
    IPAddress::ptr networkAddress(uint32_t prefix_len) override;
    IPAddress::ptr subNetmask(uint32_t prefix_len) override;

    uint32_t getPort() const override;
    void setPort(uint32_t v) override;
private:
    sockaddr_in m_addr;
};

class IPv6Address : public IPAddress{
public:
    typedef std::shared_ptr<IPv6Address> ptr;

    static IPv6Address::ptr Create(const char * address,uint32_t port = 0);
    IPv6Address();
    IPv6Address(const sockaddr_in6 & address);
    IPv6Address(const uint8_t address[16] ,uint32_t port = 0);
    ~IPv6Address(){}

    const sockaddr * getAddress()const override;
    const socklen_t getAddressLen() const override;
    std::ostream & insert(std::ostream & os) const;
    IPAddress::ptr broadcastAddress(uint32_t prefix_len) override;
    IPAddress::ptr networkAddress(uint32_t prefix_len) override;
    IPAddress::ptr subNetmask(uint32_t prefix_len) override;

    uint32_t getPort() const override;
    void setPort(uint32_t v) override;
private:
    sockaddr_in6 m_addr;
};

class UnixAddress : public Address{
public:
    typedef std::shared_ptr<UnixAddress> ptr;
    UnixAddress();
    UnixAddress(const std::string & path);

    const sockaddr * getAddress()const override;
    const socklen_t getAddressLen() const override;
    std::ostream & insert(std::ostream & os) const override;

private:
    struct sockaddr_un m_addr;
    socklen_t m_length;
};

class UnKnownAddress : public Address{
public:
    typedef std::shared_ptr<UnKnownAddress> ptr;

    UnKnownAddress();
    UnKnownAddress(const sockaddr & addr){m_addr = addr;}
    UnKnownAddress(int family);

    const sockaddr * getAddress()const override;
    const socklen_t getAddressLen() const override;
    std::ostream & insert(std::ostream & os) const override;
private:
    sockaddr m_addr;
};


}