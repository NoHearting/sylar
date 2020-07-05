/*
 * @Descripttion: 
 * @version: 
 * @Author: zsj
 * @Date: 2020-07-05 12:31:37
 * @LastEditors: zsj
 * @LastEditTime: 2020-07-05 12:49:50
 */ 
#include "sylar/module.h"
#include "sylar/singleton.h"
#include <iostream>
#include <cstdio>


class A{
public:
    A(){
        std::cout << "A::A" << this << std::endl;
    }

    ~A(){
        std::cout << "A::~A" << this << std::endl;
    }

};

class MyModule : public sylar::Module{
public:
    MyModule():Module("hello","1.0",""){
        
    }

    bool onLoad()override{
        sylar::Singleton<A>::GetInstance();
        std::cout << "----------onLoad-------------" << std::endl;
        return true;
    }

    bool onUnLoad()override{
        sylar::Singleton<A>::GetInstance();
        std::cout << "----------onUnLoad-----------" << std::endl;
        return true;
    }

};

extern "C" {

    sylar::Module * CreateModule{
        sylar::Singleton<A>::GetInstance();
        std::cout << "===========CreateModule============" << std::endl;
        return new MyModule;
    }

    void DestoryModule(sylar::Module * ptr){
        std::cout << "==========DestoryModule===========" << std::endl;
        delete ptr;
    }

}



