/*
 * @Descripttion: 
 * @version: 
 * @Author: zsj
 * @Date: 2020-06-08 18:53:51
 * @LastEditors: zsj
 * @LastEditTime: 2020-07-05 20:52:41
 */ 
#include<assert.h>
#include"../sylar/sylar.h"

// static sylar::Logger::ptr g_logger = SYLAR_LOG_NAME("system");

void test_assert(){
    SYLAR_LOG_INFO(g_logger) << sylar::BacktraceToString(10);
    SYLAR_ASSERT(false);
}

int main(int argc,char * argv[])
{
    test_assert();
}