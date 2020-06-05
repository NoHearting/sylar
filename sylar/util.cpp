/*
 * @Descripttion: 
 * @version: 
 * @Author: zsj
 * @Date: 2020-06-05 17:11:30
 * @LastEditors: zsj
 * @LastEditTime: 2020-06-05 17:21:18
 */ 
#include"util.h"
namespace sylar{

pid_t GetThreadId()
{
    return syscall(SYS_gettid);
}

uint32_t GetFiberId(){
    return 0;
}

}