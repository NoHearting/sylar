/*
 * @Descripttion: 
 * @version: 
 * @Author: zsj
 * @Date: 2020-06-12 22:55:32
 * @LastEditors: zsj
 * @LastEditTime: 2020-06-12 22:59:07
 */ 

#ifndef __SYLAR_HOOK_H__
#define __SYLAR_HOOK_H__

#include<unistd.h>
#include<time.h>

namespace sylar {
    /**
     * @brief 当前线程是否hook
     */
    bool is_hook_enable();
    /**
     * @brief 设置当前线程的hook状态
     */
    void set_hook_enable(bool flag);
}
extern "C" {

//sleep
typedef unsigned int (*sleep_fun)(unsigned int seconds);
extern sleep_fun sleep_f;

typedef int (*usleep_fun)(useconds_t usec);
extern usleep_fun usleep_f;

typedef int (*nanosleep_fun)(const struct timespec *req, struct timespec *rem);
extern nanosleep_fun nanosleep_f;

}


#endif