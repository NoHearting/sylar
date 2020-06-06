/*
 * @Descripttion: 
 * @version: 
 * @Author: zsj
 * @Date: 2020-06-04 20:39:08
 * @LastEditors: zsj
 * @LastEditTime: 2020-06-06 16:37:49
 */ 
#ifndef __SYLAR_LOG_H__
#define __SYLAR_LOG_H__

#include<string>
#include<iostream>
#include<stdint.h>
#include<memory>
#include<list>
#include<fstream>
#include<algorithm>
#include<sstream>
#include<vector>
#include<utility>
#include<map>
#include<functional>
#include<time.h>
#include<cstring>
#include<stdarg.h>

#include"singleton.h"



#define SYLAR_LOG_LEVEL(logger,level) \
    if(logger->getLevel() <= level) \
        sylar::LogEventWarp(sylar::LogEvent::ptr(new sylar::LogEvent(logger,level,__FILE__,__LINE__,0,sylar::GetThreadId(), \
            sylar::GetFiberId(),time(0)))).getSS()

#define SYLAR_LOG_DEBUG(logger) SYLAR_LOG_LEVEL(logger,sylar::LogLevel::DEBUG)
#define SYLAR_LOG_INFO(logger) SYLAR_LOG_LEVEL(logger,sylar::LogLevel::INFO)
#define SYLAR_LOG_WARN(logger) SYLAR_LOG_LEVEL(logger,sylar::LogLevel::WARN)
#define SYLAR_LOG_ERROR(logger) SYLAR_LOG_LEVEL(logger,sylar::LogLevel::ERROR)
#define SYLAR_LOG_FATAL(logger) SYLAR_LOG_LEVEL(logger,sylar::LogLevel::FATAL)

#define SYLAR_LOG_FMT_LEVEL(logger,level,fmt,...) \
    if(logger->getLevel() <= level) \
        sylar::LogEventWarp(sylar::LogEvent::ptr(new sylar::LogEvent(logger,level, \
            __FILE__,__LINE__,0,sylar::GetThreadId(),sylar::GetFiberId(),time(0)))).getEvent()->format(fmt,__VA_ARGS__)

#define SYLAR_LOG_FMT_DEBUG(logger,fmt,...) SYLAR_LOG_FMT_LEVEL(logger,sylar::LogLevel::DEBUG,fmt,__VA_ARGS__)
#define SYLAR_LOG_FMT_INFO(logger,fmt,...) SYLAR_LOG_FMT_LEVEL(logger,sylar::LogLevel::INFO,fmt,__VA_ARGS__)
#define SYLAR_LOG_FMT_WARN(logger,fmt,...) SYLAR_LOG_FMT_LEVEL(logger,sylar::LogLevel::WARN,fmt,__VA_ARGS__)
#define SYLAR_LOG_FMT_ERROR(logger,fmt,...) SYLAR_LOG_FMT_LEVEL(logger,sylar::LogLevel::ERROR,fmt,__VA_ARGS__)
#define SYLAR_LOG_FMT_FATAL(logger,fmt,...) SYLAR_LOG_FMT_LEVEL(logger,sylar::LogLevel::FATAL,fmt,__VA_ARGS__)

//获取一个默认的日志器
#define SYLAR_LOG_ROOT() sylar::LoggerMgr::GetInstance().getRoot()

namespace sylar{

class Logger;

/**
 * @brief 日志级别
 */ 
class LogLevel{
public:
    enum Level{
        DEBUG = 1,
        INFO = 2,
        WARN = 3,
        ERROR = 4,
        FATAL = 5
    };
    static const char * toString(LogLevel::Level level);
};

/**
 * @brief 日志事件
 */ 
class LogEvent{
public:
    typedef std::shared_ptr<LogEvent> ptr;
    LogEvent(std::shared_ptr<Logger> logger,LogLevel::Level level,const char * file,int32_t line,uint32_t elapse,uint32_t thread_id,uint32_t fiber_id,uint64_t time);

    const char * getFile() const {return m_file;}
    int32_t getLine() const {return m_line;}
    uint32_t getElapse()const {return m_elapse;}
    uint32_t getThreadId()const {return m_threadId;}
    uint32_t getFiberId()const {return m_fiberId;}
    uint32_t getTime()const {return m_time;}
    std::string getContent()const {return m_ss.str();}
    std::shared_ptr<Logger> getLogger() {return m_logger;}
    LogLevel::Level getLevel()const{return m_level;}
    std::stringstream & getSS() {return m_ss;}

    void format(const char * fmt,...);
    void format(const char * fmt,va_list al);
private:
    const char * m_file = nullptr;  //文件名
    int32_t m_line = 0;             //行号
    uint32_t m_elapse = 0;          //程序启动到现在的毫秒数
    uint32_t m_threadId = 0;        //线程id
    uint32_t m_fiberId = 0;         //协程id
    uint64_t m_time;                //时间戳
    std::stringstream m_ss;

    std::shared_ptr<Logger> m_logger;
    LogLevel::Level m_level;
};

class LogEventWarp{
public:
    LogEventWarp(LogEvent::ptr e);
    ~LogEventWarp();
    std::stringstream & getSS();
    LogEvent::ptr getEvent(){return m_event;}
private:
    LogEvent::ptr m_event;
};




/**
 * @brief 日志格式器
 */ 
class LogFormatter{
public:
    typedef std::shared_ptr<LogFormatter> ptr;
    LogFormatter(const std::string & pattern);

    std::string format(std::shared_ptr<Logger> logger,LogLevel::Level level,LogEvent::ptr event);
public:
    class FormatItem{
    public:
        typedef std::shared_ptr<FormatItem> ptr;
        virtual ~FormatItem(){};
        virtual void  format(std::ostream & os,std::shared_ptr<Logger> logger,LogLevel::Level level,LogEvent::ptr event) = 0;
    };

    void init();
private:
    std::string m_pattern;
    std::vector<FormatItem::ptr> m_items;
};

/**
 * @brief 日志输出地
 */ 
class LogAppender{
public:
    typedef std::shared_ptr<LogAppender> ptr;
    LogAppender(LogLevel::Level level=LogLevel::DEBUG):m_level(level){}
    virtual ~LogAppender(){}

    virtual void log(std::shared_ptr<Logger> logger,LogLevel::Level level,LogEvent::ptr event) = 0;
    void setFormatter(LogFormatter::ptr val){m_formatter = val;}
    LogFormatter::ptr getFormatter()const{return m_formatter;}

    LogLevel::Level getLevel()const{return m_level;}
    void setLevel(LogLevel::Level level){m_level = level;}
protected:
    LogLevel::Level m_level;
    LogFormatter::ptr m_formatter;
};

/**
 * @brief 日志器
 */ 
class Logger : public std::enable_shared_from_this<Logger>{
public:
    typedef std::shared_ptr<Logger> ptr;
    
    Logger(const std::string & name = "root");
    void log(LogLevel::Level level,LogEvent::ptr event);

    void debug(LogEvent::ptr event);
    void info(LogEvent::ptr event);
    void warn(LogEvent::ptr event);
    void error(LogEvent::ptr event);
    void fatal(LogEvent::ptr event);

    void addAppender(LogAppender::ptr appender);
    void delAppender(LogAppender::ptr appender);

    LogLevel::Level getLevel()const{return m_level;}
    void setLevel(LogLevel::Level level){m_level = level;}

    const std::string & getName()const{return m_name;}
private:
    std::string m_name;         //日志名称
    LogLevel::Level m_level;    //日志级别
    std::list<LogAppender::ptr> m_appenders;    // appender级别
    LogFormatter::ptr m_formatter;
};

/**
 * @brief 输出到到控制台的Appender
 */ 
class StdoutLogAppender : public LogAppender{
public:
    typedef std::shared_ptr<StdoutLogAppender> ptr;
    virtual void log(std::shared_ptr<Logger> logger,LogLevel::Level level,LogEvent::ptr event) override;

    
private:
    
};

/**
 * @brief 输出到文件的Appender
 */ 
class FileLogAppender : public LogAppender{
public:
    typedef std::shared_ptr<FileLogAppender> ptr;
    FileLogAppender(const std::string & filename);
    virtual void log(std::shared_ptr<Logger> logger,LogLevel::Level level,LogEvent::ptr event) override;

    /**
     * @brief 重新打开文件
     * @return 打开成功返回ture，失败返回false
     */ 
    bool reopen();
private:
    std::string m_filename;
    std::ofstream m_filestream;
};



class MessageFormatItem : public LogFormatter::FormatItem{
public:
    MessageFormatItem(const std::string & str = ""){}   
    void  format(std::ostream & os,std::shared_ptr<Logger> logger,LogLevel::Level level,LogEvent::ptr event) override{
        os<<event->getContent();
    }

};


class LevelFormatItem : public LogFormatter::FormatItem{
public:
    LevelFormatItem(const std::string & str = ""){}   
    void  format(std::ostream & os,std::shared_ptr<Logger> logger,LogLevel::Level level,LogEvent::ptr event) override{
        os<<LogLevel::toString(level);
    }

};

class ElapseFormatItem : public LogFormatter::FormatItem{
public:
    ElapseFormatItem(const std::string & str = ""){}   
    void  format(std::ostream & os,std::shared_ptr<Logger> logger,LogLevel::Level level,LogEvent::ptr event) override{
        os<<event->getElapse();
    }

};

class LoggerNameFormatItem : public LogFormatter::FormatItem{
public:
    LoggerNameFormatItem(const std::string & str = ""){}   
    void  format(std::ostream & os,std::shared_ptr<Logger> logger,LogLevel::Level level,LogEvent::ptr event) override{
        os<<logger->getName();
    }

};

class ThreadIdFormatItem : public LogFormatter::FormatItem{
public:
    ThreadIdFormatItem(const std::string & str = ""){}   
    void  format(std::ostream & os,std::shared_ptr<Logger> logger,LogLevel::Level level,LogEvent::ptr event) override{
        os<<event->getThreadId();
    }

};

class FiberIdFormatItem : public LogFormatter::FormatItem{
public:
    FiberIdFormatItem(const std::string & str = ""){}   
    void  format(std::ostream & os,std::shared_ptr<Logger> logger,LogLevel::Level level,LogEvent::ptr event) override{
        os<<event->getFiberId();
    }

};

class DateTimeFormatItem : public LogFormatter::FormatItem{
public:
    DateTimeFormatItem(const std::string & format = "%Y-%m-%d %H:%M:%S"):m_format(format){
        if(m_format.empty()){
            m_format = "%Y-%m-%d %H:%M:%S";
        }
       
    }
    void  format(std::ostream & os,std::shared_ptr<Logger> logger,LogLevel::Level level,LogEvent::ptr event) override{
        struct tm tm;
        time_t time = event->getTime();
        localtime_r(&time,&tm);
        char buf[64];
        strftime(buf,sizeof(buf),m_format.c_str(),&tm);
        os<<buf;
    }
private:
    std::string m_format;
};

class FilenameFormatItem : public LogFormatter::FormatItem{
public:
    FilenameFormatItem(const std::string & str = ""){}   
    void  format(std::ostream & os,std::shared_ptr<Logger> logger,LogLevel::Level level,LogEvent::ptr event) override{
        os<<event->getFile();
    }

};

class LineFormatItem : public LogFormatter::FormatItem{
public:
    LineFormatItem(const std::string & str = ""){}   
    void  format(std::ostream & os,std::shared_ptr<Logger> logger,LogLevel::Level level,LogEvent::ptr event) override{
        os<<event->getLine();
    }

};

class NewLineFormatItem : public LogFormatter::FormatItem{
public:
    NewLineFormatItem(const std::string & str = ""){}   
    void  format(std::ostream & os,std::shared_ptr<Logger> logger,LogLevel::Level level,LogEvent::ptr event) override{
        os<<std::endl;
    }

};

class StringFormatItem : public LogFormatter::FormatItem{
public:
    StringFormatItem(const std::string & str):m_string(str){}
    void  format(std::ostream & os,std::shared_ptr<Logger> logger,LogLevel::Level level,LogEvent::ptr event) override{
        os<<m_string;
    }
private:
    std::string m_string;
};


class TabFormatItem : public LogFormatter::FormatItem{
public:
    TabFormatItem(const std::string & str):m_string(str){}
    void  format(std::ostream & os,std::shared_ptr<Logger> logger,LogLevel::Level level,LogEvent::ptr event) override{
        os<<"\t";
    }
private:
    std::string m_string;
};

/**
 * @brief 日志管理器
 */ 
class LoggerManager
{
public:
    LoggerManager();
    Logger::ptr getLogger(const std::string & name);

    void init();
    Logger::ptr getRoot() const{return m_root;}
private:
    std::map<std::string,Logger::ptr> m_loggers;
    Logger::ptr m_root;
};

typedef sylar::Singleton<LoggerManager> LoggerMgr;

}


#endif