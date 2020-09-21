/*
 * @Descripttion: 
 * @version: 
 * @Author: zsj
 * @Date: 2020-07-05 20:34:48
 * @LastEditors: zsj
 * @LastEditTime: 2020-07-05 21:05:14
 */ 
#include "sylar/util.h"

#include <iostream>

int main(int argc, char ** argv) {


    std::cout << sylar::FSUtil::Mkdir("/home/zsj/workspace/work/sylar/") << std::endl;

    return 0;
}