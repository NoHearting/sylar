/*
 * @Descripttion: 
 * @version: 
 * @Author: zsj
 * @Date: 2020-06-05 19:42:43
 * @LastEditors: zsj
 * @LastEditTime: 2020-06-07 11:29:40
 */ 
#ifndef __SYLAR__SINGLETON_H__
#define __SYLAR__SINGLETON_H__
#include<memory>

namespace sylar{
    
template<typename T,typename X = void,int N = 0>
class Singleton{
public:
    static T * GetInstance(){
        static T v;
        return &v;
    }
};

template<class T,class X = void,int N = 0>
class SingletonPtr{
public:
    static std::shared_ptr<T> GetInstance(){
        static std::shared_ptr<T> v(new T);
        return v;
    }
};

}

#endif