/*
 * @Descripttion: 
 * @version: 
 * @Author: zsj
 * @Date: 2020-06-13 16:49:24
 * @LastEditors: zsj
 * @LastEditTime: 2020-06-16 11:23:05
 */ 

#include<netdb.h>
#include<utility>
#include<ifaddrs.h>
#include"address.h"
#include"log.h"
#include"util.h"

namespace sylar{

static Logger::ptr g_logger = SYLAR_LOG_NAME("system");

//normal
template<class T>
static T CreateMask(uint32_t bits){
    return (1 << (sizeof(T) * 8 - bits)) - 1;
}

template<class T>
static uint32_t CountBytes(T value){
    uint32_t result = 0;
    for(;value;++result){
        value &= value -1;
    }
    return result;
} 

//Address

Address::ptr Address::LookupAny(const std::string & host,int family,int type,int protocol){
    std::vector<Address::ptr> result;
    if(Lookup(result,host,family,type,protocol)){
        return result[0];
    }
    return nullptr;
}
IPAddress::ptr Address::LookupAnyIPAddress(const std::string & host,int family,int type,int protocol){
    std::vector<Address::ptr> result;
    if(Lookup(result,host,family,type,protocol)){
        for(auto & i : result){
            IPAddress::ptr v = std::dynamic_pointer_cast<IPAddress>(i);
            if(v){
                return v;
            }
        }
    }
    return nullptr;
}



bool Address::Lookup(std::vector<Address::ptr> & result,const std::string &host
        ,int family,int type,int protocol){
    addrinfo hints,*results,*next;
    hints.ai_flags = 0;
    hints.ai_family = family;
    hints.ai_socktype = type;
    hints.ai_protocol = protocol;
    hints.ai_addrlen = 0;
    hints.ai_canonname = nullptr;
    hints.ai_addr = nullptr;
    hints.ai_next = nullptr;

    std::string node;
    const char * service = nullptr;

    //检查 ipv6address server
    if(!host.empty() && host[0] == '['){
        const char * endipv6 = (const char * )memchr(host.c_str() + 1,']',host.size() - 1);
        //TODO check out of range
        if(*(endipv6 + 1) == ':'){
            service = endipv6 + 2;

        }
        node = host.substr(1,endipv6 - host.c_str() - 1);
    }

    //检查 node service
    if(node.empty()){
        service = (const char * )memchr(host.c_str(),':',host.size());
        if(service){
            if(!memchr(service + 1,':',host.c_str() + host.size() - service - 1)){
                node = host.substr(0,service - host.c_str());
                ++service;
            }
        }
    }

    if(node.empty()){
        node = host;
    }

    int error = getaddrinfo(node.c_str(),service,&hints,&results);
    if(error){
        SYLAR_LOG_ERROR(g_logger) << "Address::Lookup getaddress(" << host 
            << "," << family << ","<<type<<") err=" << error << " errstr="
            << strerror(errno);

            return false;
    }

    next = results;
    while(next){
        result.push_back(Create(next->ai_addr,(socklen_t)next->ai_addrlen));
        // SYLAR_LOG_INFO(g_logger) << ((sockaddr_in *)next->ai_addr)->sin_addr.s_addr;
        next = next->ai_next;
    }


    freeaddrinfo(results);
    return true;
}

bool Address::GetInterfaceAddress(std::multimap<std::string
    ,std::pair<Address::ptr,uint32_t>> & result
    ,int family){   
    struct ifaddrs * next,*results;
    if(getifaddrs(&results) != 0){
        SYLAR_LOG_ERROR(g_logger) << "Address::GetInterfaceAddress getifaddrs "
            <<" err=" << errno << " errstr="<<strerror(errno);
        return false;
    }
    try{
        for(next = results;next;next = next->ifa_next){
            Address::ptr addr;
            uint32_t prefix_length = ~0u;
            if(family != AF_UNSPEC && family != next->ifa_addr->sa_family){
                continue;
            }
            switch(next->ifa_addr->sa_family){
                case AF_INET:
                {
                    addr = Create(next->ifa_addr,sizeof(sockaddr_in));
                    uint32_t netmask = ((sockaddr_in*)next->ifa_netmask)->sin_addr.s_addr;
                    prefix_length = CountBytes(netmask);
                }
                    break;
                case AF_INET6:
                {
                    addr = Create(next->ifa_addr,sizeof(sockaddr_in6));
                    in6_addr & netmask = ((sockaddr_in6*)next->ifa_netmask)->sin6_addr;
                    prefix_length = 0;
                    for(int i = 0;i<16;i++){
                        prefix_length += CountBytes(netmask.s6_addr[i]);
                    }
                }
                    break;
                default:
                    break;

            }
            if(addr){
                result.insert(std::make_pair(next->ifa_name,
                        std::make_pair(addr,prefix_length)));
            }
        }
    }catch(...){
        SYLAR_LOG_ERROR(g_logger) << "Address::GetInterfaceAddress exception";
        freeifaddrs(results);
        return false;
    }

    freeifaddrs(results);
    return true;
}
bool Address::GetInterfaceAddress(std::vector<std::pair<Address::ptr,uint32_t>> & result
    ,const std::string & iface
    ,int family){
    if(iface.empty() || iface == ""){
        if(family == AF_INET || family == AF_UNSPEC){
            result.push_back(std::make_pair(Address::ptr(new IPv4Address()),0u));
        }
        if(family == AF_INET6 || family == AF_UNSPEC){
            result.push_back(std::make_pair(Address::ptr(new IPv6Address()),0u));
        }
        return true;
    }

    std::multimap<std::string
        ,std::pair<Address::ptr,uint32_t>>  results;
    if(!GetInterfaceAddress(results,family)){
        return false;
    }

    auto its = results.equal_range(iface);
    for(;its.first != its.second;++its.first){
        result.push_back(its.first->second);
    }
    return true;
}


Address::ptr Address::Create(const sockaddr * addr,socklen_t addrlen){
    if(addr == nullptr){
        return nullptr;
    }
    Address::ptr result;
    switch(addr->sa_family){
        case AF_INET:
            result.reset(new IPv4Address(*(sockaddr_in *)addr));
            break;
        case AF_INET6:
            result.reset(new IPv6Address(*(sockaddr_in6 *)addr));
            break;
        default:
            result.reset(new UnKnownAddress(*addr));
            break;
    }
    return result;
}


//IPAddress


IPAddress::ptr IPAddress::Create(const char * address,uint16_t port){
    addrinfo hints,*results;
    memset(&hints,0,sizeof(addrinfo));

    hints.ai_flags = AI_NUMERICHOST;
    hints.ai_family = AF_UNSPEC;

    int error = getaddrinfo(address,NULL,&hints,&results);
    if(error){
        SYLAR_LOG_ERROR(g_logger) << "IPAddress::Create(" << address 
            << ","<<port<<") error="<<error
            <<" errno="<<errno<<" errstr="<<strerror(errno);
            return nullptr;
    }   

    try{
        IPAddress::ptr result = std::dynamic_pointer_cast<IPAddress>(
            Address::Create(results->ai_addr,(socklen_t)results->ai_addrlen));
        if(result){
            result->setPort(port);
        }
        freeaddrinfo(results);
        return result;
    }catch(...){
        freeaddrinfo(results);
        return nullptr;
    }
}


int Address::getFamily() const{
    return getAddress()->sa_family;
}

std::string Address::toString() const{
    std::stringstream ss;
    insert(ss);
    return ss.str();
}

bool Address::operator<(const Address & rhs) const{
    socklen_t minlen = std::min(getAddressLen(),rhs.getAddressLen());
    int result = memcmp(getAddress(),rhs.getAddress(),minlen);
    if(result < 0){
        return true;
    }
    else if(result > 0){
        return false;
    }
    else if(getAddressLen() < rhs.getAddressLen()){
        return true;
    }
    return false;
}
bool Address::operator==(const Address & rhs) const{
    return getAddressLen() == rhs.getAddressLen()
        && memcmp(getAddress(),rhs.getAddress(),getAddressLen()) == 0;
}
bool Address::operator!=(const Address & rhs) const{
    return !(*this == rhs);
}




// IPv4Address
IPv4Address::ptr IPv4Address::Create(const char * address,uint16_t port){
    IPv4Address::ptr rt(new IPv4Address);
    rt->m_addr.sin_port = htons(port);
    int result = inet_pton(AF_INET,address,&rt->m_addr.sin_addr);
    if(result <= 0){
        SYLAR_LOG_ERROR(g_logger) << "IPv4Address::Create(" << address << ","
                                << port << ") rt="<<result <<" errno=" <<errno
                                <<" errstr=" <<strerror(errno);
                                return nullptr;
    }
    return rt;
}
    



IPv4Address::IPv4Address(sockaddr_in & addr){
    m_addr = addr;
}


IPv4Address::IPv4Address(uint32_t address,uint16_t port){
    memset(&m_addr,0,sizeof(m_addr));
    m_addr.sin_family = AF_INET;
    m_addr.sin_port = htons(port);
    m_addr.sin_addr.s_addr = htonl(address);
}
    

const sockaddr * IPv4Address::getAddress()const{
    return (sockaddr*)&m_addr;
}

sockaddr * IPv4Address::getAddress(){
    return (sockaddr*)&m_addr;
}

const socklen_t IPv4Address::getAddressLen() const{
    return sizeof(m_addr);
}
std::ostream & IPv4Address::insert(std::ostream & os) const{
    // uint32_t addr = ntohl(m_addr.sin_addr.s_addr);
    os << inet_ntoa(m_addr.sin_addr);
    os <<":"<<ntohs(m_addr.sin_port);
    return os;

}
IPAddress::ptr IPv4Address::broadcastAddress(uint32_t prefix_len){
    if(prefix_len > 32){
        return nullptr;
    }

    sockaddr_in baddr(m_addr);
    baddr.sin_addr.s_addr |= ntohl(CreateMask<uint32_t>(prefix_len));
    // IPv4Address::ptr rt(new IPv4Address(baddr.sin_addr.s_addr,))
    return IPv4Address::ptr(new IPv4Address(baddr));
}
IPAddress::ptr IPv4Address::networkAddress(uint32_t prefix_len){
    if(prefix_len > 32){
        return nullptr;
    }

    sockaddr_in baddr(m_addr);
    baddr.sin_addr.s_addr &= ntohs(CreateMask<uint32_t>(prefix_len));
    // IPv4Address::ptr rt(new IPv4Address(baddr.sin_addr.s_addr,))
    return IPv4Address::ptr(new IPv4Address(baddr));
}
IPAddress::ptr IPv4Address::subNetmask(uint32_t prefix_len){
    sockaddr_in subnet;
    memset(&subnet,0,sizeof(subnet));
    subnet.sin_family = AF_INET;
    subnet.sin_addr.s_addr = ~htonl(CreateMask<uint32_t>(prefix_len));
    return IPv4Address::ptr(new IPv4Address(subnet));
}

uint32_t IPv4Address::getPort() const{
    return ntohs(m_addr.sin_addr.s_addr);
}
void IPv4Address::setPort(uint16_t v){
    m_addr.sin_port = htons(v);
}


//IPv6Address

IPv6Address::ptr IPv6Address::Create(const char * address,uint16_t port){
    IPv6Address::ptr rt(new IPv6Address);
    rt->m_addr.sin6_port = htons(port);
    int result = inet_pton(AF_INET6,address,&rt->m_addr.sin6_addr);
    if(result <= 0){
        SYLAR_LOG_ERROR(g_logger) << "IPv6Address::Create(" << address << ","
                                << port << ") rt="<<result <<" errno=" <<errno
                                <<" errstr=" <<strerror(errno);
                                return nullptr;
    }
    return rt;
}
    

IPv6Address::IPv6Address(){
    memset(&m_addr,0,sizeof(m_addr));
    m_addr.sin6_family = AF_INET6;
}

IPv6Address::IPv6Address(const sockaddr_in6 & address){
    m_addr = address;
}

IPv6Address::IPv6Address(const uint8_t address[16] ,uint16_t port){
    memset(&m_addr,0,sizeof(m_addr));
    m_addr.sin6_family = AF_INET6;
    m_addr.sin6_port = htons(port);
    memcpy(&m_addr.sin6_addr.s6_addr,address,16);
}

const sockaddr * IPv6Address::getAddress()const{
    return (sockaddr*)&m_addr;
}



const socklen_t IPv6Address::getAddressLen() const{
    return sizeof(m_addr);
}

std::ostream & IPv6Address::insert(std::ostream & os) const{
    os << "[";
    uint16_t* addr = (uint16_t*)m_addr.sin6_addr.s6_addr;
    bool used_zeros = false;
    for(size_t i = 0; i < 8; ++i) {
        if(addr[i] == 0 && !used_zeros) {
            continue;
        }
        if(i && addr[i - 1] == 0 && !used_zeros) {
            os << ":";
            used_zeros = true;
        }
        if(i) {
            os << ":";
        }
        os << std::hex << (int)ntohl(addr[i]) << std::dec;
    }

    if(!used_zeros && addr[7] == 0) {
        os << "::";
    }

    os << "]:" << ntohs(m_addr.sin6_port);
    return os;

}

uint32_t IPv6Address::getPort() const{
    return ntohs(m_addr.sin6_port);
}
void IPv6Address::setPort(uint16_t v){
    m_addr.sin6_port = htons(v);
}

IPAddress::ptr IPv6Address::broadcastAddress(uint32_t prefix_len){
    sockaddr_in6 baddr(m_addr);
    baddr.sin6_addr.s6_addr[prefix_len / 8] |= CreateMask<uint8_t>(prefix_len % 8);
    for(int i = prefix_len / 8 + 1;i < 16;++i){
        baddr.sin6_addr.s6_addr[i] = 0xff;
    }
    return IPv6Address::ptr(new IPv6Address(baddr));
}
IPAddress::ptr IPv6Address::networkAddress(uint32_t prefix_len){
    sockaddr_in6 baddr(m_addr);
    baddr.sin6_addr.s6_addr[prefix_len / 8] &= CreateMask<uint8_t>(prefix_len % 8);
    // for(int i = prefix_len / 8 + 1;i < 16;++i){
    //     baddr.sin6_addr.s6_addr[i] = 0xff;
    // }
    return IPv6Address::ptr(new IPv6Address(baddr));
}
IPAddress::ptr IPv6Address::subNetmask(uint32_t prefix_len){
    sockaddr_in6 subnet;
    memset(&subnet,0,sizeof(subnet));
    subnet.sin6_family = AF_INET6;
    subnet.sin6_addr.s6_addr[prefix_len / 8] = ~CreateMask<uint8_t>(prefix_len % 8);
    for(uint32_t i = 0;i < prefix_len / 8;++i){
        subnet.sin6_addr.s6_addr[i] = 0xff;
    }
    return IPv6Address::ptr(new IPv6Address(subnet));
}


//UnixAddress
static const size_t MAX_PATH_LEN = sizeof(((sockaddr_un*)0)->sun_path) - 1;

UnixAddress::UnixAddress(){
    memset(&m_addr,0,sizeof(m_addr));
    m_addr.sun_family = AF_UNIX;
    m_length = offsetof(sockaddr_un,sun_path) + MAX_PATH_LEN;
}
UnixAddress::UnixAddress(const std::string & path){
    memset(&m_addr,0,sizeof(m_addr));
    m_addr.sun_family = AF_UNIX;
    m_length = path.size() + 1;
    if(!path.empty() && path[0] == '\0'){
        --m_length;
    }
    if(m_length > sizeof(m_addr.sun_path)){
        throw std::logic_error("path to long");
    }
    memcpy(m_addr.sun_path,path.c_str(),m_length);
    m_length += offsetof(sockaddr_un,sun_path);
}

const sockaddr * UnixAddress::getAddress()const{
    return (sockaddr*)&m_addr;
}
const socklen_t UnixAddress::getAddressLen() const {
    return m_length;
}

std::ostream & UnixAddress::insert(std::ostream & os) const {
    if(m_length > offsetof(sockaddr_un,sun_path)
        && m_addr.sun_path[0] == '\0'){
        return os << "\\0" << std::string(m_addr.sun_path + 1,m_length - offsetof(sockaddr_un,sun_path) - 1);
    }
    return os << m_addr.sun_path;
}


//UnkownAddress

UnKnownAddress::UnKnownAddress(int family){
    memset(&m_addr,0,sizeof(m_addr));
    m_addr.sa_family = family;
}

const sockaddr * UnKnownAddress::getAddress()const{
    return &m_addr;
}
const socklen_t UnKnownAddress::getAddressLen() const{
    return sizeof(m_addr);
}
std::ostream & UnKnownAddress::insert(std::ostream & os) const{
    os << "[UnknowAddress family=" << m_addr.sa_family << "]";
    return os;
}

std::ostream & operator<<(std::ostream & os,const Address & addr){
    return addr.insert(os);
}


}