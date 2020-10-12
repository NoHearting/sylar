/*
 * @Descripttion:
 * @version:
 * @Author: zsj
 * @Date: 2020-10-12 16:50:35
 * @LastEditors: zsj
 * @LastEditTime: 2020-10-12 17:03:06
 */
#include "sylar/log.h"
#include "sylar/streams/zlib_stream.h"
#include "sylar/util.h"

static sylar::Logger::ptr g_logger = SYLAR_LOG_ROOT();

void test_gzip() {
    SYLAR_LOG_INFO(g_logger) << "===================================gzip======="
                                "=====================";
    std::string data = sylar::random_string(102400);
    auto gzip_compress = sylar::ZlibStream::CreateGzip(true, 1);
    SYLAR_LOG_INFO(g_logger)
        << "compress: " << gzip_compress->write(data.c_str(), data.size())
        << " length: " << gzip_compress->getBuffers().size();
    SYLAR_LOG_INFO(g_logger) << "flush: " << gzip_compress->flush();
    auto compress_str = gzip_compress->getResult();
    auto gzip_uncompress = sylar::ZlibStream::CreateGzip(false, 1);

    SYLAR_LOG_INFO(g_logger)
        << "uncompress: "
        << gzip_compress->write(compress_str.c_str(), compress_str.size())
        << " length: " << gzip_uncompress->getBuffers().size();
    SYLAR_LOG_INFO(g_logger) << "flush: " << gzip_uncompress->flush();
    auto uncompress_str = gzip_uncompress->getResult();
    SYLAR_LOG_INFO(g_logger) << "test_gzip:" << (data == uncompress_str)
                             << " origin_size: " << data.size()
                             << " uncompress.size: " << uncompress_str.size();
}

void test_deflate() {
    SYLAR_LOG_INFO(g_logger)
        << "===================================deflate======="
           "=====================";
    std::string data = sylar::random_string(102400);
    auto gzip_compress = sylar::ZlibStream::CreateDeflate(true, 1);
    SYLAR_LOG_INFO(g_logger)
        << "compress: " << gzip_compress->write(data.c_str(), data.size())
        << " length: " << gzip_compress->getBuffers().size();
    SYLAR_LOG_INFO(g_logger) << "flush: " << gzip_compress->flush();
    auto compress_str = gzip_compress->getResult();
    auto gzip_uncompress = sylar::ZlibStream::CreateDeflate(false, 1);

    SYLAR_LOG_INFO(g_logger)
        << "uncompress: "
        << gzip_compress->write(compress_str.c_str(), compress_str.size())
        << " length: " << gzip_uncompress->getBuffers().size();
    SYLAR_LOG_INFO(g_logger) << "flush: " << gzip_uncompress->flush();
    auto uncompress_str = gzip_uncompress->getResult();
    SYLAR_LOG_INFO(g_logger) << "test_gzip:" << (data == uncompress_str)
                             << " origin_size: " << data.size()
                             << " uncompress.size: " << uncompress_str.size();
}

void test_zlib() {
    SYLAR_LOG_INFO(g_logger) << "===================================zlib======="
                                "=====================";
    std::string data = sylar::random_string(102400);
    auto gzip_compress = sylar::ZlibStream::CreateZlib(true, 1);
    SYLAR_LOG_INFO(g_logger)
        << "compress: " << gzip_compress->write(data.c_str(), data.size())
        << " length: " << gzip_compress->getBuffers().size();
    SYLAR_LOG_INFO(g_logger) << "flush: " << gzip_compress->flush();
    auto compress_str = gzip_compress->getResult();
    auto gzip_uncompress = sylar::ZlibStream::CreateZlib(false, 1);

    SYLAR_LOG_INFO(g_logger)
        << "uncompress: "
        << gzip_compress->write(compress_str.c_str(), compress_str.size())
        << " length: " << gzip_uncompress->getBuffers().size();
    SYLAR_LOG_INFO(g_logger) << "flush: " << gzip_uncompress->flush();
    auto uncompress_str = gzip_uncompress->getResult();
    SYLAR_LOG_INFO(g_logger) << "test_gzip:" << (data == uncompress_str)
                             << " origin_size: " << data.size()
                             << " uncompress.size: " << uncompress_str.size();
}

int main(int argc, char** argv) {
    srand(time(0));
    test_gzip();
    test_deflate();
    test_zlib();
    return 0;
}