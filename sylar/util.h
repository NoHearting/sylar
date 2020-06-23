/*
 * @Descripttion: 
 * @version: 
 * @Author: zsj
 * @Date: 2020-06-05 17:08:39
 * @LastEditors: zsj
 * @LastEditTime: 2020-06-22 22:01:30
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


//时间
uint64_t GetCurrentMS();
uint64_t GetCurrentUS();

std::string TimeToString(time_t ts = time(0),const std::string & format = "%Y-%m-%d %H:%M:%S");


class FSUtil{
public:
    static void listAllFile(std::vector<std::string> & files
                            ,const std::string & path
                            ,const std::string & subfix);
    

    static bool Mkdir(const std::string& dirname);
    static bool IsRunningPidfile(const std::string& pidfile);

};

}

#endif
