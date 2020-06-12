/*
 * @Descripttion: 
 * @version: 
 * @Author: zsj
 * @Date: 2020-06-12 22:59:38
 * @LastEditors: zsj
 * @LastEditTime: 2020-06-12 23:02:45
 */ 
#include"hook.h"
#include"log.h"

sylar::Logger::ptr g_logger = SYLAR_LOG_NAME("system");
namespace sylar{



#define HOOK_FUN(XX) \
    XX(sleep) \
    XX(usleep) \
    XX(nanosleep) 





}