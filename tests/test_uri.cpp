/*
 * @Descripttion: 
 * @version: 
 * @Author: zsj
 * @Date: 2020-06-21 11:10:48
 * @LastEditors: zsj
 * @LastEditTime: 2020-06-21 11:18:41
 */ 
#include"../sylar/uri.h"
#include<iostream>



int main(int argc,char ** argv){
    sylar::Uri::ptr uri = sylar::Uri::Create("http://www.xl-zsj.top/test/uri?id=100&name=sylar&vv=中文#frg");
    std::cout<<uri->toString()<<std::endl;
    auto addr = uri->createAddress();
    std::cout << *addr <<std::endl;

}