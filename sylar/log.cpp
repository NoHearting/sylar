/*
 * @Descripttion: 
 * @version: 
 * @Author: zsj
 * @Date: 2020-06-04 22:47:54
 * @LastEditors: zsj
 * @LastEditTime: 2020-06-05 19:42:11
 */ 
#include"log.h"
namespace sylar{


const char * LogLevel::toString(LogLevel::Level level){
    switch(level){
#define XX(name) \
    case LogLevel::name: \
        return #name; \
        break;
    
    XX(DEBUG);
    XX(INFO);
    XX(WARN);
    XX(ERROR);
    XX(FATAL);
#undef XX
    default:
        return "UNKOWN";
        break;
    }
    return "UNKOWN";
}

LogEventWarp::LogEventWarp(LogEvent::ptr e):m_event(e){

}
LogEventWarp::~LogEventWarp(){
    m_event->getLogger()->log(m_event->getLevel(),m_event);
}
std::stringstream & LogEventWarp::getSS(){
    return m_event->getSS();
}

void LogEvent::format(const char * fmt,...){
    va_list al;
    va_start(al,fmt);
    format(fmt,al);
    va_end(al);
}
void LogEvent::format(const char * fmt,va_list al){
    char *buf = nullptr;
    int len = vasprintf(&buf,fmt,al);
    if(len != -1){
        m_ss << std::string(buf,len);
        free(buf);
    }
}

LogEvent::LogEvent(std::shared_ptr<Logger> logger,LogLevel::Level level,const char * file
    ,int32_t line,uint32_t elapse,uint32_t thread_id,uint32_t fiber_id,uint64_t time)
    :m_file(file),m_line(line),m_elapse(elapse),m_threadId(thread_id)
    ,m_fiberId(fiber_id),m_time(time),m_logger(logger),m_level(level)
{

}

Logger::Logger(const std::string & name)
    :m_name(name),m_level(LogLevel::DEBUG)
{
    m_formatter.reset(new LogFormatter("%d{%Y-%m-%d %H:%M:%S}%T%t%T%F%T[%p]%T[%c]%T%f:%l%T%m%n"));
}

void Logger::addAppender(LogAppender::ptr appender){
    if(!appender->getFormatter()){
        appender->setFormatter(m_formatter);
    }
    m_appenders.push_back(appender);
}
void Logger::delAppender(LogAppender::ptr appender){
    for(auto iter = m_appenders.begin();iter!=m_appenders.end();++iter){
        if(*iter == appender){
            m_appenders.erase(iter);
            break;
        }
    }
}

void Logger::log(LogLevel::Level level,LogEvent::ptr event){
    if(level >= m_level){
        auto self = shared_from_this();
        for(auto & iter : m_appenders){
            iter->log(self,level,event);
        }
    }
}

void Logger::debug(LogEvent::ptr event){
    log(LogLevel::DEBUG,event);
}
void Logger::info(LogEvent::ptr event){
    log(LogLevel::DEBUG,event);
}
void Logger::warn(LogEvent::ptr event){
    log(LogLevel::DEBUG,event);
}
void Logger::error(LogEvent::ptr event){
    log(LogLevel::DEBUG,event);
}
void Logger::fatal(LogEvent::ptr event){
    log(LogLevel::DEBUG,event);
}

void StdoutLogAppender::log(std::shared_ptr<Logger> logger,LogLevel::Level level,LogEvent::ptr event){
    if(level >= m_level){
        std::cout<<m_formatter->format(logger,level,event);
    }
}

bool FileLogAppender::reopen(){
    if(m_filestream){
        m_filestream.close();
    }
    m_filestream.open(m_filename);
    return !m_filestream;
}

FileLogAppender::FileLogAppender(const std::string & filename):m_filename(filename){
    reopen();
}

void FileLogAppender::log(std::shared_ptr<Logger> logger,LogLevel::Level level,LogEvent::ptr event){
    if(level >= m_level){
        m_filestream<<m_formatter->format(logger,level,event);
    }
}


LogFormatter::LogFormatter(const std::string & pattern):m_pattern(pattern){
    init();
}

std::string LogFormatter::format(std::shared_ptr<Logger> logger,LogLevel::Level level,LogEvent::ptr event){
    std::stringstream ss;
    for(auto & iter : m_items){
        iter->format(ss,logger,level,event);
    }
    return ss.str();
}
void LogFormatter::init(){
    //str,format,type
    std::vector<std::tuple<std::string,std::string,int>>vec;
    std::string nstr;
    for(size_t i = 0;i<m_pattern.size();++i){
        if(m_pattern[i] != '%'){
            nstr.append(1,m_pattern[i]);
            continue;
        }

        if(i+1 < m_pattern.size()){
            if(m_pattern[i+1] == '%'){
                nstr.append(1,'%');
                continue;
            }
        }

        size_t n = i + 1;
        int fmt_status = 0;
        size_t fmt_begin = 0;
        std::string str;
        std::string fmt;
        while(n < m_pattern.size()){
            if(!fmt_status && !isalpha(m_pattern[n]) && m_pattern[n] != '{' 
                && m_pattern[n] != '}'){
                str = m_pattern.substr(i+1,n-i-1);
                break;
            }
            if(fmt_status == 0){
                if(m_pattern[n] == '{'){
                    str = m_pattern.substr(i+1,n-i-1);
                    fmt_status = 1;  // 解析格式
                    fmt_begin = n;
                    ++n;
                    continue;
                }
            }
            if(fmt_status == 1){
                if(m_pattern[n] == '}'){
                    fmt = m_pattern.substr(fmt_begin + 1,n-fmt_begin-1);
                    fmt_status = 0;
                    ++n;
                    break;
                }
            }
            n++;  
            if(n == m_pattern.size()){
                if(str.empty()){
                    str = m_pattern.substr(i+1);
                }
            }
        }

        if(fmt_status == 0){
            if(!nstr.empty()){
                vec.push_back(std::make_tuple(nstr,"",0));
                nstr.clear();
            }
            // str = m_pattern.substr(i+1,n-i-1);
            vec.push_back(std::make_tuple(str,fmt,1));
            i = n - 1;
        }
        else if(fmt_status == 1){
            std::cout<<"pattern parse error: "<<m_pattern<<" - "<<m_pattern.substr(i)<<std::endl;
            vec.push_back(std::make_tuple("<<pattern_error>>",fmt,0));
        }
    }

    if(!nstr.empty()){
        vec.push_back(std::make_tuple(nstr,"",0));
    }

    /**
     * %m  -- 日志消息
     * %p  -- 日志级别
     * %r  -- 启动后的毫秒数
     * %c  -- 日志名称
     * %t  -- 线程id
     * %n  -- 回车换行
     * %d  -- 时间格式
     * %f  -- 文件名
     * %l  -- 行号
     */
    static std::map<std::string,std::function<FormatItem::ptr(const std::string & str)> > s_format_items = {

#define XX(str,C) \
        {#str,[](const std::string & fmt) {return FormatItem::ptr(new C(fmt));} }

        XX(m,MessageFormatItem),
        XX(p,LevelFormatItem),
        XX(r,ElapseFormatItem),
        XX(c,LoggerNameFormatItem),
        XX(t,ThreadIdFormatItem),
        XX(n,NewLineFormatItem),
        XX(d,DateTimeFormatItem),
        XX(f,FilenameFormatItem),
        XX(l,LineFormatItem),
        XX(T,TabFormatItem),
        XX(F,FiberIdFormatItem),

#undef XX
    };
    for(auto & iter : vec){
        if(std::get<2>(iter) == 0){
            m_items.push_back(LogFormatter::FormatItem::ptr(new StringFormatItem(std::get<0>(iter))));
        }
        else{
            auto it = s_format_items.find(std::get<0>(iter));
            if(it == s_format_items.end()){
                m_items.push_back(LogFormatter::FormatItem::ptr(new StringFormatItem("<<error_format %"+std::get<0>(iter) + ">>")));
            }
            else{
                m_items.push_back(it->second(std::get<1>(iter)));
            }
        }
        // std::cout<<"("<<std::get<0>(iter)<< ") - ("<<std::get<1>(iter) <<") - (" << std::get<2>(iter) <<")"<< std::endl;
    }


}

LoggerManager::LoggerManager()
{
    m_root.reset(new Logger);
    m_root->addAppender(LogAppender::ptr(new StdoutLogAppender));
}
Logger::ptr LoggerManager::getLogger(const std::string & name)
{
    if(m_loggers.find(name)!=m_loggers.end()){
        return m_loggers[name];
    }
    else{
        return m_root;
    }
}

void LoggerManager::init(){
    
}


















}

