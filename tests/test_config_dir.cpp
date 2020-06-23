/*
 * @Descripttion: 
 * @version: 
 * @Author: zsj
 * @Date: 2020-06-23 13:23:02
 * @LastEditors: zsj
 * @LastEditTime: 2020-06-23 13:31:50
 */ 
#include"../sylar/log.h"
#include"../sylar/config.h"
#include"../sylar/env.h"
#include<iostream>



static sylar::Logger::ptr g_logger = SYLAR_LOG_ROOT();

int main(int argc, char *argv[])
{
    SYLAR_LOG_DEBUG(g_logger) << "begin log!!  --------";
    SYLAR_LOG_ERROR(g_logger) << "begin log!!  --------";

    sylar::EnvMgr::GetInstance()->init(argc,argv);
    sylar::Config::LoadFromConfDir("conf");

    SYLAR_LOG_DEBUG(g_logger) << "end log!! ******";
    SYLAR_LOG_ERROR(g_logger) << "end log!! ******";


    sylar::Config::Visit([](sylar::ConfigVarBase::ptr var){
        std::cout<< "name="<<var->getName()
            <<" description="<<var->getDescription()
            <<" typename="<<var->getTypeName()
            <<" value=" << var->toString()<<std::endl;
    });

    return 0;
}
