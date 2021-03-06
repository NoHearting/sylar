/*
 * @Descripttion: 
 * @version: 
 * @Author: zsj
 * @Date: 2020-06-15 18:49:46
 * @LastEditors: zsj
 * @LastEditTime: 2020-06-18 09:05:47
 */ 
#pragma once

#include"http.h"
#include"httpclient_parser.h"
#include"http11_parser.h"

namespace sylar
{
namespace http
{


class HttpRequestParser{
public:
    typedef std::shared_ptr<HttpRequestParser> ptr;

    HttpRequestParser();

    size_t execute(char * data,size_t len);
    int isFinished();
    int hasError();

    HttpRequest::ptr getData()const{return m_data;}
    void setError(int error){m_error = error;}

    uint64_t getContentLength();

    const http_parser & getParser()const {return m_parser;}
public:
    static uint64_t GetHttpRequestBufferSize();
    static uint64_t GetHttpRequestMaxBodySize();
private:
    http_parser m_parser;
    HttpRequest::ptr m_data;

    //1000:invalid method
    //1001:invalid version
    //1002:invalid field
    int m_error;

};

class HttpResponseParser{
public:
    typedef std::shared_ptr<HttpResponseParser> ptr;
    HttpResponseParser();

    size_t execute(char * data,size_t len,bool chunck);
    int isFinished() ;
    int hasError();
    void setError(int error){m_error = error;}

    HttpResponse::ptr getData()const{return m_data;}
    uint64_t getContentLength();

    const httpclient_parser & getParser()const{
        return m_parser;
    }

public:
    static uint64_t GetHttpResponseBufferSize();
    static uint64_t GetHttpResponseMaxBodySize();
private:
    httpclient_parser m_parser;
    HttpResponse::ptr m_data;
    //1001:invalid version
    //1002:invalid field
    int m_error;
};

}
}