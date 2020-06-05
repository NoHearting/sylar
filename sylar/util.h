/*
 * @Descripttion: 
 * @version: 
 * @Author: zsj
 * @Date: 2020-06-05 17:08:39
 * @LastEditors: zsj
 * @LastEditTime: 2020-06-05 17:22:13
 */ 
#ifndef __SYLAR__UTIL_H__
#define __SYLAR__UTIL_H__

#include<pthread.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/syscall.h>
#include<stdint.h>


namespace sylar{
 
pid_t GetThreadId();
uint32_t GetFiberId();

}

#endif
