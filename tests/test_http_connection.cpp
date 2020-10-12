/*
 * @Descripttion:
 * @version:
 * @Author: zsj
 * @Date: 2020-06-17 22:12:56
 * @LastEditors: zsj
 * @LastEditTime: 2020-10-12 18:08:59
 */
#include <iostream>

#include "sylar/http/http_connection.h"
#include "sylar/http/http_parser.h"
#include "sylar/iomanager.h"
#include "sylar/log.h"
#include "sylar/streams/zlib_stream.h"

static sylar::Logger::ptr g_logger = SYLAR_LOG_ROOT();

void test_pool() {
    sylar::http::HttpConnectionPool::ptr pool(
        new sylar::http::HttpConnectionPool("www.sylar.top", "", 80, false, 10,
                                            1000 * 30, 5));
    sylar::IOManager::GetThis()->addTimer(
        1000,
        [pool]() {
            auto r = pool->doGet("/", 300);
            SYLAR_LOG_INFO(g_logger) << r->toString();
        },
        true);
}


void test_https() {
    auto r = sylar::http::HttpConnection::DoGet(
        "https://www.baidu.com/", 300,
        {{"Accept-Encodeing", "gzip,deflate,br"},
         {"Connection", "keep-alive"},
         {"User-Agent", "curl/7.29.0"}});

    SYLAR_LOG_INFO(g_logger)
        << "result=" << r->result << " error=" << r->error
        << " rsp=" << (r->response ? r->response->toString() : "");
    // sylar::http::HttpConnectionPool::ptr pool(
    //     new sylar::http::HttpConnectionPool("www.baidu.com", "", 443, true,
    //     10,
    //                                         1000 * 30, 5));
    auto pool = sylar::http::HttpConnectionPool::Create(
        "https://www.baidu.com/", "", 10, 1000 * 30, 5);
    sylar::IOManager::GetThis()->addTimer(
        1000,
        [pool]() {
            auto r = pool->doGet("/", 300,
                                 {{"Accept-Encoding", "gzip,deflate,br"},
                                  {"User-Agent", "curl/7.29.0"}});
            SYLAR_LOG_INFO(g_logger) << r->toString();
        },
        true);
}

void test_data() {
    sylar::Address::ptr addr = sylar::Address::LookupAny("www.baidu.com:80");
    auto sock = sylar::Socket::CreateTCP(addr);
    sock->connect(addr);
    const char buff[] =
        "GET / HTTP/1.1\r\nconnection: close\r\nAccept-Encoding: gzip, "
        "deflate, br\r\nHost: www.baidu.com\r\n\r\n";
    sock->send(buff, sizeof(buff));
    std::string line;
    line.resize(1024);
    std::ofstream ofs("http.dat", std::ios::binary);
    int total = 0;
    int len = 0;
    while ((len = sock->recv(&line[0], line.size())) > 0) {
        total += len;
        ofs.write(line.c_str(), len);
    }
    SYLAR_LOG_INFO(g_logger) << "total: " << total << " tellp=" << ofs.tellp();
    ofs.flush();
}


void test_parser() {
    std::ifstream ifs("http.dat", std::ios::binary);
    std::string content;
    std::string line;
    line.resize(1024);

    int total = 0;
    while (!ifs.eof()) {
        ifs.read(&line[0], line.size());
        content.append(&line[0], ifs.gcount());
        total += ifs.gcount();
    }

    SYLAR_LOG_INFO(g_logger)
        << "length:" << content.size() << " total: " << total;
    sylar::http::HttpResponseParser parser;
    size_t nparse = parser.execute(&content[0], content.size(), false);
    SYLAR_LOG_INFO(g_logger) << "finish: " << parser.isFinished();
    content.resize(content.size() - nparse);
    SYLAR_LOG_INFO(g_logger) << "rsp: " << *parser.getData();
    auto& client_parser = parser.getParser();
    std::string body;
    int cl = 0;
    do {
        size_t nparse = parser.execute(&content[0], content.size(), true);
        SYLAR_LOG_INFO(g_logger) << "content_len:" << client_parser.content_len
                                 << " left: " << content.size();
        cl += client_parser.content_len;
        content.resize(content.size() - nparse);
        body.append(content.c_str(), client_parser.content_len);
        content = content.substr(client_parser.content_len + 2);

    } while (!client_parser.chunks_done);

    SYLAR_LOG_INFO(g_logger) << "total: " << body.size() << " content:" << cl;
    sylar::ZlibStream::ptr stream = sylar::ZlibStream::CreateGzip(false);
    stream->write(body.c_str(), body.size());
    stream->flush();

    body = stream->getResult();
    std::ofstream ofs("http.txt");
    ofs << body << std::endl;
}

void run() {
    sylar::Address::ptr addr =
        sylar::Address::LookupAnyIPAddress("www.xl-zsj.top:80");
    if (!addr) {
        SYLAR_LOG_INFO(g_logger) << "get addr error";
        return;
    }

    sylar::Socket::ptr sock = sylar::Socket::CreateTCP(addr);
    bool rt = sock->connect(addr);
    if (!rt) {
        SYLAR_LOG_INFO(g_logger) << "connect " << *addr << " failed";
        return;
    }
    sylar::http::HttpConnection::ptr conn(
        new sylar::http::HttpConnection(sock));
    sylar::http::HttpRequest::ptr req(new sylar::http::HttpRequest);
    req->setHeader("Host", "www.baidu.com");
    SYLAR_LOG_INFO(g_logger) << "req:" << std::endl << *req;

    conn->sendRequest(req);
    auto rsp = conn->recvResponse();
    if (!rsp) {
        SYLAR_LOG_INFO(g_logger) << "recv response error";
        return;
    }

    // SYLAR_LOG_INFO(g_logger) << "rsp:"<<std::endl<<*rsp;


    SYLAR_LOG_INFO(g_logger) << "========================================";
    auto rt2 = sylar::http::HttpConnection::DoGet("http://www.baidu.com/", 300);
    SYLAR_LOG_INFO(g_logger)
        << "result=" << rt2->result << " error=" << rt2->error
        << " rsp=" << (rt2->response ? rt2->response->toString() : "");
    test_pool();
}


int main(int argc, char** argv) {
    sylar::IOManager iom(2);
    iom.schedule(test_parser);
}