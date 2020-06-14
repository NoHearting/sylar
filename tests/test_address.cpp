/*
 * @Descripttion: 
 * @version: 
 * @Author: zsj
 * @Date: 2020-06-14 13:42:05
 * @LastEditors: zsj
 * @LastEditTime: 2020-06-14 14:07:52
 */ 
#include"../sylar/address.h"
#include"../sylar/log.h"

static sylar::Logger::ptr g_logger = SYLAR_LOG_ROOT();

void test(){
    std::vector<sylar::Address::ptr> addrs;
    SYLAR_LOG_INFO(g_logger) << "begin";
    bool v = sylar::Address::Lookup(addrs,"xl-zsj.top");
    SYLAR_LOG_INFO(g_logger) << "end";
    if(!v){
        SYLAR_LOG_ERROR(g_logger) << "lookup fail";
        return;
    }

    for(size_t i = 0;i<addrs.size();i++){
        SYLAR_LOG_INFO(g_logger) << i <<" - "<<addrs[i]->toString();
    }
}

void test_iface(){
    std::multimap<std::string,std::pair<sylar::Address::ptr,uint32_t>> results;
    bool v = sylar::Address::GetInterfaceAddress(results);
    if(!v){
        SYLAR_LOG_ERROR(g_logger) << "GetInterfaceAddress fail";
        return;
    }
    for(auto & i : results){
        SYLAR_LOG_INFO(g_logger) << i.first <<" - "<<i.second.first->toString()<<" - "
            << i.second.second;
    }
}

void test_ipv4(){
    // auto addr = sylar::IPAddress::Create("www.xl-zsj.top");
    auto addr = sylar::IPAddress::Create("127.0.0.8");
    if(addr){
        SYLAR_LOG_INFO(g_logger) << addr->toString();
    }
}

int main(int argc,char **argv)
{
    // test();  
    // test_iface();
    test_ipv4();
    
}
