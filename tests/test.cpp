/*
 * @Descripttion: 
 * @version: 
 * @Author: zsj
 * @Date: 2020-06-05 14:59:11
 * @LastEditors: zsj
 * @LastEditTime: 2020-06-05 19:59:06
 */ 

#include<iostream>
#include"../sylar/log.h"
#include"../sylar/util.h"
#include"../sylar/singleton.h"


int main(int argc,char * argv[])
{
    sylar::Logger::ptr logger(new sylar::Logger);
    logger->addAppender(sylar::LogAppender::ptr(new sylar::StdoutLogAppender));


    // sylar::LogEvent::ptr event(new sylar::LogEvent(__FILE__,__LINE__,0,sylar::GetThreadId(),sylar::getFiberId(),time(0)));
    // event->getSS()<<"hello sylar logger";
    // logger->log(sylar::LogLevel::DEBUG,event);

    sylar::LogAppender::ptr file_appender(new sylar::FileLogAppender("./log.txt"));
    sylar::LogFormatter::ptr file_formatter(new sylar::LogFormatter("%d%T%p%T%m%n"));

    file_appender->setFormatter(file_formatter);
    file_appender->setLevel(sylar::LogLevel::ERROR);

    logger->addAppender(file_appender);

    std::cout<<"log test"<<std::endl;

    SYLAR_LOG_INFO(logger) << "test macro";

    SYLAR_LOG_FMT_ERROR(logger,"test macro fmt warn %s","xx");
    SYLAR_LOG_ERROR(logger)<<"test macro error";

    auto l = sylar::LoggerMgr::GetInstance().getLogger("XX");
    SYLAR_LOG_INFO(l)<<"XXX";
    
}