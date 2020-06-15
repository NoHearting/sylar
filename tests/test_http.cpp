/*
 * @Descripttion: 
 * @version: 
 * @Author: zsj
 * @Date: 2020-06-15 15:12:10
 * @LastEditors: zsj
 * @LastEditTime: 2020-06-15 15:22:44
 */ 
#include"../sylar/http/http.h"
#include"../sylar/log.h"
#include<iostream>


void test_request(){
    sylar::http::HttpRequest::ptr req(new sylar::http::HttpRequest);
    req->setHeader("host","www.xl-zsj.top");
    req->setBody("hello zsj");
    req->dump(std::cout) << std::endl;
}

void test_response(){
    sylar::http::HttpResponse::ptr rsp(new sylar::http::HttpResponse);
    rsp->setHeader("X-X","sylar");
    rsp->setBody("hello sylar");
    rsp->setClose(false);
    rsp->setStatus(sylar::http::HttpStatus::NOT_FOUND);
    rsp->dump(std::cout)<<std::endl;
}

int main(int argc,char ** argv){
    test_request();
    test_response();
}