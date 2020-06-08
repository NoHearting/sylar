/*
 * @Descripttion: 
 *  - 定义日志系统，主要的组件有：
 *      - 日志器（Logger）
 *      - 日志输出器（LogAppender），及其子类
 *          - StdoutLogAppender，定义日志打印到文件
 *          - FileLogAppender，定义到日志打印到控制台（标准输出）
 *      - 日志级别类（LogLevel），5个日志级别，一个错误级别
 *          - UNKOWN 0
 *          - DEBUG 1
 *          - INFO  2
 *          - WARN  3
 *          - ERROR 4
 *          - FATAL 5
 *      - 日志事件类（LogEvent）
 *      - 日志事件包装类（LogEventWarp）
 *      - 日志格式器类（LogFormatter），及其内部类的子类
 *      - 日志器管理类（LoggerManager），为单例类
 *  - 日志系统主要工作逻辑：
 *      - 打印日志首先使用LogEventWarp（其实是使用的里面的LogEvent），然后到LoggerManager取出一个Logger
 *      - 向取出的Logger中添加LogAppener，同时设置Logger和LogAppener的LogFormatter和LogLevel
 *      - 开始打印日志
 *  - 日志系统和配置系统的整合：
 *      - 配置系统整合所包含的内容在[log.cpp]
 *  
 * @version: 
 * @Author: zsj
 * @Date: 2020-06-04 20:39:08
 * @LastEditors: zsj
 * @LastEditTime: 2020-06-07 21:58:57
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
#include<set>
#include<functional>
#include<time.h>
#include<cstring>
#include<stdarg.h>
#include<yaml-cpp/yaml.h>

#include"singleton.h"


//定义宏用于日志的输出，使用流的方式
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

//定义宏用于日志的输出，使用参数列表的方式（类似于printf）
#define SYLAR_LOG_FMT_DEBUG(logger,fmt,...) SYLAR_LOG_FMT_LEVEL(logger,sylar::LogLevel::DEBUG,fmt,__VA_ARGS__)
#define SYLAR_LOG_FMT_INFO(logger,fmt,...) SYLAR_LOG_FMT_LEVEL(logger,sylar::LogLevel::INFO,fmt,__VA_ARGS__)
#define SYLAR_LOG_FMT_WARN(logger,fmt,...) SYLAR_LOG_FMT_LEVEL(logger,sylar::LogLevel::WARN,fmt,__VA_ARGS__)
#define SYLAR_LOG_FMT_ERROR(logger,fmt,...) SYLAR_LOG_FMT_LEVEL(logger,sylar::LogLevel::ERROR,fmt,__VA_ARGS__)
#define SYLAR_LOG_FMT_FATAL(logger,fmt,...) SYLAR_LOG_FMT_LEVEL(logger,sylar::LogLevel::FATAL,fmt,__VA_ARGS__)

//获取一个默认的日志器
#define SYLAR_LOG_ROOT() sylar::LoggerMgr::GetInstance()->getRoot()

//根据name获取一个Logger
#define SYLAR_LOG_NAME(name) sylar::LoggerMgr::GetInstance()->getLogger(name)

namespace sylar{

class Logger;
class LoggerManager;

/**
 * @brief 日志级别
 *  - 用于标志日志的级别，5个可用级别和一个不可用级别（UNKOWN）
 *  - 提供日志级别和字符串的相互转化
 */ 
class LogLevel{
public:
    enum Level{
        UNKNOW = 0,
        DEBUG = 1,
        INFO = 2,
        WARN = 3,
        ERROR = 4,
        FATAL = 5
        
    };
    static const char * toString(LogLevel::Level level);
    static LogLevel::Level FromString(const std::string & str);
};

/**
 * @brief 日志事件
 *  - 日志事件会使用日志器来进行日志的输出
 *  - 日志事件包含日志的所有信息，
 *      - 包括日志的时间，日志的输出线程id、协程id，日志级别，如果要写入到文件，则包含文件名
 *  - 日志事件的日志级别需要大于日志器中所定义的日志级别才能正确写日志
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
    std::stringstream m_ss;         //日志的输入流

    std::shared_ptr<Logger> m_logger;
    LogLevel::Level m_level;
};

/**
 * @brief sylar::LogEvent的包装类，主要用于输出日志
 *  - 由于本日志系统使用的是定义宏来进行一个日志输出，所以使用本类的析构函数进行打印日志
 */ 
class LogEventWarp{
public:
    LogEventWarp(LogEvent::ptr e);

    /**
     * @brief 析构的时候进行打印日志，在宏中被构建
     */ 
    ~LogEventWarp();
    std::stringstream & getSS();
    LogEvent::ptr getEvent(){return m_event;}
private:
    LogEvent::ptr m_event;  //日志事件
};




/**
 * @brief 日志格式器，将日志格式化为特定的格式
 *  - 包含一个内部纯虚类（FormatItem），用于被子类继承实现不同的格式化部分
 *  - 可以格式化的格式：
 *      - %m  -- 日志消息
 *      - %p  -- 日志级别
 *      - %r  -- 启动后的毫秒数
 *      - %c  -- 日志名称
 *      - %t  -- 线程id
 *      - %n  -- 回车换行
 *      - %d  -- 时间格式
 *      - %f  -- 文件名
 *      - %l  -- 行号
 *      - %T  -- tab
 *      - %F  -- 协程id
 *      -     -- 普通字符
 */ 
class LogFormatter{
public:
    typedef std::shared_ptr<LogFormatter> ptr;
    LogFormatter(const std::string & pattern);

    /**
     * @brief 格式化需要打印的日志并返回
     *  - 具体由负责不同部分的格式化器共同格式化并组合返回而成
     */ 
    std::string format(std::shared_ptr<Logger> logger,LogLevel::Level level,LogEvent::ptr event);
public:

    /**
     * @brief 纯虚类
     *  - 更加具体的格式化类，用于格式化整体中的一部分
     */ 
    class FormatItem{
    public:
        typedef std::shared_ptr<FormatItem> ptr;
        virtual ~FormatItem(){};

        /**
         * @brief 格式化具体的部分到参数os所指向的流中
         */ 
        virtual void  format(std::ostream & os,std::shared_ptr<Logger> logger,LogLevel::Level level,LogEvent::ptr event) = 0;
    };

    void setError(bool error){m_error = error;}
    bool isError()const{return m_error;}
    std::string getPattern()const{return m_pattern;}

    /**
     * @brief 解析格式字符串m_pattern，将需要格式化的格式分离保存，
     *      - 并将具体的格式化类初始化到m_items中
     */ 
    void init();
private:
    std::string m_pattern;                 //格式化的格式
    std::vector<FormatItem::ptr> m_items;  //日志格式化器需要格式化的每一项

    bool m_error = false;  //格式是否错误
};

/**
 * @brief 日志输出地，用于将日志打印到具体的位置（stdout or file）
 *  - 只有日志打印级别大于LogAppender设置的日志级别才能被正常打印
 */ 
class LogAppender{
friend class Logger;
public:
    typedef std::shared_ptr<LogAppender> ptr;
    LogAppender(LogLevel::Level level=LogLevel::DEBUG):m_level(level){}
    virtual ~LogAppender(){}

    //打印日志
    virtual void log(std::shared_ptr<Logger> logger,LogLevel::Level level,LogEvent::ptr event) = 0;
    
    /**
     * @brief 设置日志打印的formatter,如果不设置则使用Logger的formatter
     */ 
    void setFormatter(LogFormatter::ptr val){
        m_formatter = val;
        m_hasFormatter = m_formatter ? true : false;
    }
    LogFormatter::ptr getFormatter()const{return m_formatter;}

    LogLevel::Level getLevel()const{return m_level;}
    void setLevel(LogLevel::Level level){m_level = level;}
    
    /**
     * @brief 将本类属性转化为yaml格式，使用yaml-cpp库
     */ 
    virtual std::string toYamlString() = 0;

protected:
    LogLevel::Level m_level;            //打印的日志需要达到的日志级别
    LogFormatter::ptr m_formatter;      //日志格式化器
    bool m_hasFormatter = false;        //判断是否有日志格式化器，没有则用Logger的日志格式化器
};

/**
 * @brief 日志器
 * - 用于控制日志的输出
 * - 继承于std::enable_shared_from_this<Logger>
 *      - 主要是为了使用shared_from_this()方法取得自身的智能指针
 * - 其包含属性的默认值为：
 *      - name=root
 *      - level=DEBUG
 *      - fomat=%d{%Y-%m-%d %H:%M:%S}%T%t%T%F%T[%p]%T[%c]%T%f:%l%T%m%n
 *      - appender(输出器)=无默认输出器，需要后续添加
 * - 含有一个友元类：LoggerManager
 */ 
class Logger : public std::enable_shared_from_this<Logger>{

friend class LoggerManager;  

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
    void clearAppenders(){m_appenders.clear();}

    LogLevel::Level getLevel()const{return m_level;}
    void setLevel(LogLevel::Level level){ m_level = level; }

    const std::string & getName()const{return m_name;}

    void setFormatter(LogFormatter::ptr val){
        m_formatter = val;
        for(auto & i : m_appenders){
            if(!i->m_hasFormatter){
                i->m_formatter = m_formatter;
            }
        }
    }
    void setFormatter(const std::string & val){
        sylar::LogFormatter::ptr new_val(new sylar::LogFormatter(val));
        if(new_val->isError()){
            std::cout<<"Logger setFormatter name = " << val
                     << " value=" << val << " invalid formatter"
                     << std::endl;
            return ;
        }
        setFormatter(new_val);
    }
    LogFormatter::ptr getFormatter()const{return m_formatter;}

    /**
     * @brief 将本类转化为yaml形式的字符串，使用yaml-cpp库
     */ 
    std::string toYamlString();
private:
    std::string m_name;         //日志名称
    LogLevel::Level m_level;    //日志级别
    std::list<LogAppender::ptr> m_appenders;    // 日志输出器
    LogFormatter::ptr m_formatter;

    Logger::ptr m_root;
};

/**
 * @brief 输出到到控制台的Appender，将日志输出到控制台
 */ 
class StdoutLogAppender : public LogAppender{
public:
    typedef std::shared_ptr<StdoutLogAppender> ptr;
    virtual void log(std::shared_ptr<Logger> logger,LogLevel::Level level,LogEvent::ptr event) override;

    std::string toYamlString() override;
private:
    
};

/**
 * @brief 输出到文件的Appender，将日志输出到文件，需要指定文件名
 */ 
class FileLogAppender : public LogAppender{
public:
    typedef std::shared_ptr<FileLogAppender> ptr;
    FileLogAppender(const std::string & filename);
    virtual void log(std::shared_ptr<Logger> logger,LogLevel::Level level,LogEvent::ptr event) override;

    /**
     * @brief 重新打开文件
     *  - 如果文件已经打开，则先关闭再打开
     *  - 如果文件不存在，则新建并写入，如果文件存在，则以追加的方式写入文件
     * @return 打开成功返回ture，失败返回false
     */ 
    bool reopen();


    std::string toYamlString()override;
private:
    std::string m_filename;         //文件名
    std::ofstream m_filestream;     //文件流
};



/**
 * @brief 格式化【消息】的格式化类，用于格式化【消息（%m）】子项
 */ 
class MessageFormatItem : public LogFormatter::FormatItem{
public:
    MessageFormatItem(const std::string & str = ""){}   
    void  format(std::ostream & os,std::shared_ptr<Logger> logger,LogLevel::Level level,LogEvent::ptr event) override{
        os<<event->getContent();
    }

};

/**
 * @brief 格式化【日志等级】的格式化类，用于格式化【日志等级（%p）】子项
 */ 
class LevelFormatItem : public LogFormatter::FormatItem{
public:
    LevelFormatItem(const std::string & str = ""){}   
    void  format(std::ostream & os,std::shared_ptr<Logger> logger,LogLevel::Level level,LogEvent::ptr event) override{
        os<<LogLevel::toString(level);
    }

};

/**
 * @brief 格式化【时间戳】的格式化类，用于格式化【时间戳（%r）】子项
 */ 
class ElapseFormatItem : public LogFormatter::FormatItem{
public:
    ElapseFormatItem(const std::string & str = ""){}   
    void  format(std::ostream & os,std::shared_ptr<Logger> logger,LogLevel::Level level,LogEvent::ptr event) override{
        os<<event->getElapse();
    }

};

/**
 * @brief 格式化【日志器的名称】的格式化类，用于格式化【日志器名称（%c）】子项
 */ 
class LoggerNameFormatItem : public LogFormatter::FormatItem{
public:
    LoggerNameFormatItem(const std::string & str = ""){}   
    void  format(std::ostream & os,std::shared_ptr<Logger> logger,LogLevel::Level level,LogEvent::ptr event) override{
        os<<event->getLogger()->getName();
    }

};

/**
 * @brief 格式化【线程id】的格式化类，用于格式化【线程id（%m）】子项
 */ 
class ThreadIdFormatItem : public LogFormatter::FormatItem{
public:
    ThreadIdFormatItem(const std::string & str = ""){}   
    void  format(std::ostream & os,std::shared_ptr<Logger> logger,LogLevel::Level level,LogEvent::ptr event) override{
        os<<event->getThreadId();
    }

};


/**
 * @brief 格式化【协程id】的格式化类，用于格式化【协程id（%F）】子项
 */ 
class FiberIdFormatItem : public LogFormatter::FormatItem{
public:
    FiberIdFormatItem(const std::string & str = ""){}   
    void  format(std::ostream & os,std::shared_ptr<Logger> logger,LogLevel::Level level,LogEvent::ptr event) override{
        os<<event->getFiberId();
    }

};

/**
 * @brief 格式化【时间】的格式化类，用于格式化【时间（%d）】子项
 */ 
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


/**
 * @brief 格式化【文件名】的格式化类，用于格式化【文件名（%f）】子项
 */ 
class FilenameFormatItem : public LogFormatter::FormatItem{
public:
    FilenameFormatItem(const std::string & str = ""){}   
    void  format(std::ostream & os,std::shared_ptr<Logger> logger,LogLevel::Level level,LogEvent::ptr event) override{
        os<<event->getFile();
    }

};

/**
 * @brief 格式化【日志所在行数】的格式化类，用于格式化【日志所在行数（%l）】子项
 */ 
class LineFormatItem : public LogFormatter::FormatItem{
public:
    LineFormatItem(const std::string & str = ""){}   
    void  format(std::ostream & os,std::shared_ptr<Logger> logger,LogLevel::Level level,LogEvent::ptr event) override{
        os<<event->getLine();
    }

};

/**
 * @brief 格式化【换行】的格式化类，用于格式化【换行（%n）】子项
 */ 
class NewLineFormatItem : public LogFormatter::FormatItem{
public:
    NewLineFormatItem(const std::string & str = ""){}   
    void  format(std::ostream & os,std::shared_ptr<Logger> logger,LogLevel::Level level,LogEvent::ptr event) override{
        os<<std::endl;
    }

};

/**
 * @brief 格式化【普通字符】的格式化类，用于格式化【普通字符】子项
 */ 
class StringFormatItem : public LogFormatter::FormatItem{
public:
    StringFormatItem(const std::string & str):m_string(str){}
    void  format(std::ostream & os,std::shared_ptr<Logger> logger,LogLevel::Level level,LogEvent::ptr event) override{
        os<<m_string;
    }
private:
    std::string m_string;
};

/**
 * @brief 格式化【tab】的格式化类，用于格式化【tab（%T）】子项
 */ 
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
 * @brief 日志器管理类，用于管理所有的日志器
 *  - 默认持有一个root日志器
 */ 
class LoggerManager
{
public:
    LoggerManager();

    /**
     * @brief 根据名字获取一个日志器（logger）
     *  - 如果有则取出，如果没有则新建并返回
     */ 
    Logger::ptr getLogger(const std::string & name);

    void init();
    Logger::ptr getRoot() const{return m_root;}

    /**
     * @brief 用于将本类的属性转化为yaml格式，使用yaml-cpp库
     */ 
    std::string toYamlString();
private:
    std::map<std::string,Logger::ptr> m_loggers;  //存放当前系统中所有的日志器
    Logger::ptr m_root;  //基本的日志器
};

typedef sylar::Singleton<LoggerManager> LoggerMgr;

}


#endif