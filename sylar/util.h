/*
 * @Descripttion: 
 * @version: 
 * @Author: zsj
 * @Date: 2020-06-05 17:08:39
 * @LastEditors: zsj
 * @LastEditTime: 2020-06-10 15:28:26
 */ 
#ifndef __SYLAR__UTIL_H__
#define __SYLAR__UTIL_H__

#include<pthread.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/syscall.h>
#include<stdint.h>
#include<vector>
#include<string>



namespace sylar{
 
pid_t GetThreadId();
uint32_t GetFiberId();
void Backtrace(std::vector<std::string> & bt,int size = 64,int skip = 1);
std::string BacktraceToString(int size = 64,int skip = 2,const std::string & prefix = "");

}

#endif
