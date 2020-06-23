/*
 * @Descripttion: 
 * @version: 
 * @Author: zsj
 * @Date: 2020-06-21 19:29:30
 * @LastEditors: zsj
 * @LastEditTime: 2020-06-23 09:28:45
 */ 
#pragma once

#include<functional>
#include<unistd.h>
#include<string>
#include"singleton.h"

namespace sylar
{

struct ProcessInfo{
    /// 父进程id
    pid_t parent_id;

    ///子进程id
    pid_t main_id;

    ///子进程启动事件
    uint64_t parent_start_time = 0;

    /// 主进程启动事件
    uint64_t main_start_time = 0;

    ///主进程重启的事件
    uint32_t restart_count = 0;

    std::string toString()const;
};

typedef sylar::Singleton<ProcessInfo> ProcessInfoMgr;


/**
 * @brief 启动程序可以选择用守护进程的方式
 * @param argc 参数个数
 * @param argv 参数值数组
 * @param main_cb 启动函数
 * @param is_daemon 是否守护进程
 * @return int 程序执行结果
 */ 
int start_daemon(int argc,char ** argv
        ,std::function<int(int,char**)> main_cb,bool is_daemon);

} // namespace sylar
