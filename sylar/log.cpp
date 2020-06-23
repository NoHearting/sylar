/*
 * @Descripttion: 
 * - 日志系统注释见[log.h]
 * - 配置系统的整合信息。配置系统主要组件：
 *      - 日志输出器定义（LogAppenderDefine）
 *      - 日志器定义（LogDefine）
 *      - 转化器（将yaml配置转化为日志器定义/日志输出器定义）,用于配置的序列化和反序列化
 *          - LexicalCast<std::string,std::set<LogDefine> >, string to set<LogDefine>
 *          - LexicalCast<std::set<LogDefine>,std::string >, set<LogDefine> to string
 *      - 日志初始化器（LogIniter）
 *  - 配置系统生效的逻辑：
 *      - 可以进行一些配置的初始化（即默认配置）
 *      - 然后读取配置文件中的配置，将文件中的配置项全部读取保存
 *          - 会使用LexicalCast<std::string,std::set<LogDefine> >将配置读取并转化为set数据结构
 *      - 将保存的配置项和现有配置进行对比，
 *          - 如果有则覆盖默认配置，即修改日志配置（主要是修改日志器Logger）
 *          - 如果没有默认配置，则会创建配置项对应的数据结构，即创建日志配置（主要是创建日志器Logger）
 * @version: 
 * @Author: zsj
 * @Date: 2020-06-04 22:47:54
 * @LastEditors: zsj
 * @LastEditTime: 2020-06-23 15:49:06
 */ 
#include"log.h"
#include"config.h"
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

LogLevel::Level LogLevel::FromString(const std::string & str){

#define XX(level,level_str) \
    if(str == #level_str) { \
        return LogLevel::level; \
    }

    XX(DEBUG,debug);
    XX(INFO,info);
    XX(WARN,warn);
    XX(ERROR,error);
    XX(FATAL,fatal);

    XX(DEBUG,DEBUG);
    XX(INFO,INFO);
    XX(WARN,WARN);
    XX(ERROR,ERROR);
    XX(FATAL,FATAL);
    
    return LogLevel::UNKNOW;
#undef XX
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

LogEvent::LogEvent(std::shared_ptr<Logger> logger,LogLevel::Level level
        ,const char * file,int32_t line,uint32_t elapse
        ,uint32_t thread_id,uint32_t fiber_id,uint64_t time
        ,const std::string & thread_name
        )
    :m_file(file)
    ,m_line(line)
    ,m_elapse(elapse)
    ,m_threadId(thread_id)
    ,m_fiberId(fiber_id)
    ,m_time(time)
    ,m_threadName(thread_name)
    ,m_logger(logger)
    ,m_level(level)
    
    
{

}

Logger::Logger(const std::string & name)
    :m_name(name),m_level(LogLevel::DEBUG)
{
    m_formatter.reset(new LogFormatter("%d{%Y-%m-%d %H:%M:%S}%T%t%T%N%T%F%T[%p]%T[%c]%T%f:%l%T%m%n"));
}

void Logger::addAppender(LogAppender::ptr appender){
    MutexType::Lock lock(m_mutex);
    if(!appender->getFormatter()){
        MutexType::Lock ll(appender->m_mutex);
        appender->m_formatter = m_formatter;
    }
    m_appenders.push_back(appender);
}
void Logger::delAppender(LogAppender::ptr appender){
    MutexType::Lock lock(m_mutex);
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
        MutexType::Lock lock(m_mutex);
        if(!m_appenders.empty()){
            for(auto & iter : m_appenders){
                iter->log(self,level,event);
            }
        }
        else if(m_root){
            m_root->log(level,event);
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

std::string Logger::toYamlString(){
    MutexType::Lock lock(m_mutex);
    YAML::Node node;
    node["name"] = m_name;
    if(m_level != LogLevel::UNKNOW){
        node["level"] = LogLevel::toString(m_level);
    }
    
    if(m_formatter){
        node["formatter"] = m_formatter->getPattern();
    }
    for(auto & i : m_appenders){
        node["appenders"].push_back(YAML::Load(i->toYamlString()));
    }
    std::stringstream ss;
    ss << node;
    return ss.str();
}

void StdoutLogAppender::log(std::shared_ptr<Logger> logger,LogLevel::Level level,LogEvent::ptr event){
    if(level >= m_level){
        MutexType::Lock lock(m_mutex);
        std::cout<<m_formatter->format(logger,level,event);
    }
}

std::string StdoutLogAppender::toYamlString(){
    MutexType::Lock lock(m_mutex);
    YAML::Node node;
    node["type"] = "StdoutLogAppender";
    if(m_level != LogLevel::UNKNOW){
        node["level"] = LogLevel::toString(m_level);
    }
    
    if(m_hasFormatter && m_formatter){
        node["formatter"] = m_formatter->getPattern();
    }
    std::stringstream ss;
    ss<<node;
    return ss.str();
}

bool FileLogAppender::reopen(){
    MutexType::Lock lock(m_mutex);
    if(m_filestream){
        m_filestream.close();
    }
    m_filestream.open(m_filename,std::ios_base::out|std::ios_base::app);
    if(!m_filestream.is_open()){
        m_filestream.open(m_filename,std::ios_base::out);
    }
    return !m_filestream;
}


std::string FileLogAppender::toYamlString(){
    YAML::Node node;
    node["type"] = "FileLogAppender";
    node["file"] = m_filename;
    if(m_level != LogLevel::UNKNOW){
        node["level"] = LogLevel::toString(m_level);
    }
    
    if(m_formatter && m_hasFormatter){
        node["formatter"] = m_formatter->getPattern();
    }
    std::stringstream ss;
    ss<<node;
    return ss.str();
}

FileLogAppender::FileLogAppender(const std::string & filename):m_filename(filename){
    reopen();
}

void FileLogAppender::log(std::shared_ptr<Logger> logger,LogLevel::Level level,LogEvent::ptr event){
    if(level >= m_level){
        uint64_t now = time(0);
        if(now != m_lastTime){
            reopen();
            m_lastTime = now;
        }
        MutexType::Lock lock(m_mutex);
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
            m_error = true;
            vec.push_back(std::make_tuple("<<pattern_error>>",fmt,0));
        }
    }

    if(!nstr.empty()){
        vec.push_back(std::make_tuple(nstr,"",0));
    }
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
        XX(N,ThreadNameFormatItem),

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
                m_error = true;
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

    m_loggers[m_root->m_name] = m_root;
    init();
}
Logger::ptr LoggerManager::getLogger(const std::string & name)
{
    MutexType::Lock lock(m_mutex);
    if(m_loggers.find(name)!=m_loggers.end()){
        return m_loggers[name];
    }
    else{
        Logger::ptr logger(new Logger(name));
        logger->m_root = m_root;
        m_loggers[name] = logger;
        return logger;
    }
}

void LoggerManager::init(){
    
}



std::string LoggerManager::toYamlString(){
    MutexType::Lock lock(m_mutex);
    YAML::Node node;
    for(auto & i : m_loggers){
        node.push_back(YAML::Load(i.second->toYamlString()));
    }
    std::stringstream ss;
    ss << node;
    return ss.str();
}


/**
 * @brief 日志输出器定义类，用于yaml字符串和日志输出类的序列化和反序列化 
 */ 
struct  LogAppenderDefine{
    int type = 0; // 1 File ,2 Stdout
    LogLevel::Level level = LogLevel::UNKNOW;
    std::string formatter;
    std::string file;

    bool operator==(const LogAppenderDefine & oth) const{
        return type == oth.type
            && level == oth.level
            && formatter == oth.formatter
            && file == oth.file;
    }

};

/**
 * @brief 日志器定义，用于yaml字符串和日志器的序列和反序列话
 */ 
struct LogDefine {
    std::string name;
    LogLevel::Level level = LogLevel::UNKNOW;
    std::string formatter;
    std::vector<LogAppenderDefine> appenders;

    bool operator==(const LogDefine & oth) const{
        return name == oth.name
            && level == oth.level
            && formatter == oth.formatter
            && appenders == oth.appenders;
    }

    bool operator<(const LogDefine & oth) const{
        return name < oth.name;
    }
};


/**
 * @brief LexicalCast类的一个偏特化
 * - std::string to std::set<LogDefine>
 */ 
template<>
class LexicalCast<std::string,std::set<LogDefine> >{
public:
    std::set<LogDefine> operator()(const std::string & v){
        YAML::Node node = YAML::Load(v);
        std::set<LogDefine> vec;
        for(size_t i = 0;i<node.size();++i){
            auto n = node[i];
            if(!n["name"].IsDefined()){
                std::cout<<"log config error: name is null"<<n<<std::endl;
                continue;
            }
            LogDefine ld;

            ld.name = n["name"].as<std::string>();
            ld.level = LogLevel::FromString((n["level"].IsDefined() ? n["level"].as<std::string>() : ""));
            if(n["formatter"].IsDefined()){
                ld.formatter = n["formatter"].as<std::string>();
            }
            if(n["appenders"].IsDefined()){
                for(size_t x = 0;x < n["appenders"].size();++x){
                    auto a = n["appenders"][x];
                    if(!a["type"].IsDefined()){
                        std::cout<<"log config error: appender type is null"<<a<<std::endl;
                        continue;
                    }
                    std::string type = a["type"].as<std::string>();
                    LogAppenderDefine lad;
                    if(type == "FileLogAppender"){
                        lad.type = 1;
                        if(!a["file"].IsDefined()){
                            std::cout<<"log config error: fileappender file is null "<<a<<std::endl;
                            continue;
                        }
                        lad.file = a["file"].as<std::string>();
                        if(a["formatter"].IsDefined()){
                            lad.formatter = a["formatter"].as<std::string>();
                        }
                    }
                    else if(type == "StdoutLogAppender"){
                        lad.type = 2;
                    }
                    else{
                        std::cout<<"log config error: appender type is invalid,"<<a<<std::endl;
                        continue;
                    }

                    ld.appenders.push_back(lad);

                }
            }
            vec.insert(ld);
        }
        return vec;
    }
};

/**
 * @brief LexicalCast类的一个偏特化
 *  - std::set<LogDefine> to std::string
 */ 
template<>
class LexicalCast<std::set<LogDefine>,std::string>{
public:
    std::string operator()(const std::set<LogDefine> & v){
        YAML::Node node;
        for(auto & i : v){
            YAML::Node n;
            n["name"] = i.name;
            if(i.level != LogLevel::UNKNOW){
                n["level"] = LogLevel::toString(i.level);
            }
            if(!i.formatter.empty()){
                n["formatter"] = i.formatter;
            }

            for(auto & a : i.appenders){
                YAML::Node na;
                if(a.type == 1){
                    na["type"] = "FileLogAppender";
                    na["file"] = a.file;
                }
                else if(a.type == 2){
                    na["type"] = "StdoutLogAppender";
                }

                if(!a.formatter.empty()){
                    na["formatter"] = a.formatter;
                }
                if(a.level != LogLevel::UNKNOW){
                    na["level"] = LogLevel::toString(a.level);
                }
                n["appenders"].push_back(na);
            }
            node.push_back(n);
        }
        std::stringstream ss;
        ss<<node;
        return ss.str();
    }
};


sylar::ConfigVar<std::set<LogDefine> >::ptr g_log_defines = 
    sylar::Config::Lookup("logs",std::set<LogDefine>(),"log config");


/**
 * @brief 日志初始化类。
 *  - 添加一个监听器，用于监听默认配置是否改变，若改变则更新配置并更新日志器
 */ 
struct LogIniter{
    LogIniter(){

        g_log_defines->addListener([](const std::set<LogDefine> & old_value,
                const std::set<LogDefine> & new_value)
        {
            SYLAR_LOG_INFO(SYLAR_LOG_NAME("system")) << "on_logger_conf_changed";
            // std::cout<<"--------------begin old_value --------------"<<std::endl;
            // for(auto & item : old_value){
            //     std::cout << " name=" << item.name << std::endl
            //               << " level=" << LogLevel::toString(item.level) << std::endl
            //               << " formatter=" << item.formatter << std::endl;
            //     for(auto & appender : item.appenders){
            //         std::cout << " \tappender:" << std::endl
            //                   << "\t\ttype=" << appender.type<<std::endl
            //                   << "\t\tlevel="<< LogLevel::toString(appender.level) << std::endl
            //                   << "\t\tformatter=" << appender.formatter<<std::endl
            //                   << "\t\tfile" << appender.file << std::endl;
            //     }
            // }
            // std::cout << "==============end old_value============" <<std::endl;

            // std::cout<<"--------------begin new_value --------------"<<std::endl;
            // for(auto & item : new_value){
            //     std::cout << " name=" << item.name << std::endl
            //               << " level=" << LogLevel::toString(item.level) << std::endl
            //               << " formatter=" << item.formatter << std::endl;
            //     for(auto & appender : item.appenders){
            //         std::cout << " \tappender:" << std::endl
            //                   << "\t\ttype=" << appender.type<<std::endl
            //                   << "\t\tlevel="<< LogLevel::toString(appender.level) << std::endl
            //                   << "\t\tformatter=" << appender.formatter<<std::endl
            //                   << "\t\tfile" << appender.file << std::endl;
            //     }
            // }
            // std::cout << "=============end old_value=============" <<std::endl;

            //新增或者修改
            for(auto & item : new_value){
                auto it = old_value.find(item);
                sylar::Logger::ptr logger;

                if(it == old_value.end()){
                    //新增logger
                    logger = SYLAR_LOG_NAME(item.name);
                    
                }
                else{
                    if(!(item == *it)){
                        //修改logger
                        logger = SYLAR_LOG_NAME(item.name);
                    }else{
                        continue;
                    }
                }
                logger->setLevel(item.level);
                if(!item.formatter.empty()){
                    logger->setFormatter(item.formatter);
                }
                logger->clearAppenders();
                for(auto & a : item.appenders){
                    sylar::LogAppender::ptr ap;
                    if(a.type == 1){
                        ap.reset(new FileLogAppender(a.file));
                    }
                    else if(a.type == 2){
                        ap.reset(new StdoutLogAppender);
                    }
                    ap->setLevel(a.level);
                    if(!a.formatter.empty()){
                        LogFormatter::ptr fmt(new LogFormatter(a.formatter));
                        if(!fmt->isError()){
                            ap->setFormatter(fmt);
                        }
                        else{
                            std::cout<<"logger name="<<item.name<<"appender type="<<a.type
                                     << "formatter="<<a.formatter
                                     <<" is invalid"<<std::endl;
                        }
                        

                    }
                    logger->addAppender(ap);
                }
            }
            for(auto & item : old_value){
                auto it = new_value.find(item);
                if(it == new_value.end()){
                    //删除logger
                    auto logger = SYLAR_LOG_NAME(item.name);
                    logger->setLevel((LogLevel::Level)100);
                    logger->clearAppenders();
                }
            }

        });
    }
};

//用于在main函数之前进行初始化
static LogIniter __log_init;


















}

