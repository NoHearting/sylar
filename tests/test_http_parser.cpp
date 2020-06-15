/*
 * @Descripttion: 
 * @version: 
 * @Author: zsj
 * @Date: 2020-06-15 22:14:38
 * @LastEditors: zsj
 * @LastEditTime: 2020-06-15 23:08:55
 */ 
#include"../sylar/http/http_parser.h"
#include"../sylar/log.h"


sylar::Logger::ptr g_logger = SYLAR_LOG_ROOT();

const char test_request_data[] = "GET / HTTP/1.1\r\n"
                                "Host: www.xl-zsj.top\r\n"
                                "Content-Length: 10\r\n\r\n"
                                "1234567890";

void test_request(){
    sylar::http::HttpRequestParser parser;
    std::string temp = test_request_data;
    size_t s = parser.execute(&temp[0],temp.size());
    SYLAR_LOG_INFO(g_logger) << "execute rt = " <<s 
        << " has_error="<<parser.hasError()
        << " is_finished="<<parser.isFinished()
        << " total size="<<temp.size()
        << " content length=" << parser.getContentLength();
    
    temp.resize(temp.size() - s);
    SYLAR_LOG_INFO(g_logger) << parser.getData()->toString();
    SYLAR_LOG_INFO(g_logger) << temp;
    // parser.getData()->dump(std::cout)<<std::endl;
}


const char test_response_data[] = "HTTP/1.1 200 OK\r\n"
                                "Accept-Ranges: bytes\r\n"
                                "Cache-Control: no-cache\r\n"
                                "Connection: keep-alive\r\n"
                                "Content-Length: 14615\r\n"
                                "Content-Type: text/html\r\n"
                                "Date: Mon, 15 Jun 2020 14:56:01 GMT\r\n"
                                "Pragma: no-cache\r\n"
                                "Server: BWS/1.1\r\n\r\n"
                                "<html>\r\n"
                                "begin\r\n"
                                "</html>\r\n";
void test_response(){
    sylar::http::HttpResponseParser parser;
    std::string temp = test_response_data;
    size_t rt = parser.execute(&temp[0],temp.size());
    SYLAR_LOG_INFO(g_logger) << "execute rt" << rt  
        << " has_error="<<parser.hasError()
        << " is_finished="<<parser.isFinished()
        << " total size="<<temp.size()
        << " content length=" << parser.getContentLength();
    temp.resize(temp.size() - rt);

    SYLAR_LOG_INFO(g_logger) << parser.getData()->toString();
    SYLAR_LOG_INFO(g_logger) << temp;
}

int main(int agrc,char ** argv)
{
    test_request();
    SYLAR_LOG_INFO(g_logger) <<"------------------";
    test_response();

}