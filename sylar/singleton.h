/*
 * @Descripttion: 
 * @version: 
 * @Author: zsj
 * @Date: 2020-06-05 19:42:43
 * @LastEditors: zsj
 * @LastEditTime: 2020-07-05 14:35:50
 */ 
#ifndef __SYLAR__SINGLETON_H__
#define __SYLAR__SINGLETON_H__
#include<memory>

namespace sylar{
    
namespace {


template<class T,class X,int N>
T & GetInstanceX(){
    static T v;
    return v;
}

template<class T,class X,int N>
std::shared_ptr<T> GetInstacncePtr(){
    static std::shared_ptr<T> v(new T);
    return v;
}

}


template<typename T,typename X = void,int N = 0>
class Singleton{
public:
    static T * GetInstance(){
        // return &GetInstanceX<T,X,N>();
        static T v;
        return &v;
    }
};

template<class T,class X = void,int N = 0>
class SingletonPtr{
public:
    static std::shared_ptr<T> GetInstance(){
        // return GetInstacncePtr<T,X,N>();
        static std::shared_ptr<T> v(new T);
        return v;
    }
};

}

#endif