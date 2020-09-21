/*
 * @Descripttion: 
 * @version: 
 * @Author: zsj
 * @Date: 2020-06-22 08:54:24
 * @LastEditors: zsj
 * @LastEditTime: 2020-07-05 22:09:13
 */ 
#include"env.h"
#include"log.h"
#include<cstring>
#include<iomanip>
#include<iostream>
#include<unistd.h>
#include<cstdlib>


static sylar::Logger::ptr g_logger = SYLAR_LOG_NAME("system");
namespace sylar
{

bool Env::init(int argc,char ** argv){

    char link[1024] = {0};
    char path[1024] = {0};
    sprintf(link,"/proc/%d/exe",getpid());


    /**
     * @brief 将参数path的符号链接内容存储到参数buf所指的内存空间
     * ssize_t readlink(const char* path1, har* path2 , size_t size );
     * @param[in] path1 文件路径
     * @param[out] path2 path1符号链接所链接到的内容
     * @param[in] path2的长度。若小于path1链接的内容的长度，则会被截断
     * @return 成功返回字符数，失败返回-1
     */     
    readlink(link,path,sizeof(path));  
    m_exe = path; //程序可执行文件的路径
    
    auto pos = m_exe.find_last_of("/");
    m_cwd = m_exe.substr(0,pos) + "/";

    SYLAR_LOG_DEBUG(g_logger) << "m_exe=" << m_exe << "  m_cwd="<<m_cwd; 

    m_program = argv[0];
    const char * now_key = nullptr;
    for(int i = 1;i<argc;i++){
        if(argv[i][0] == '-'){
            if(strlen(argv[i]) > 1){
                if(now_key != nullptr){
                    add(now_key,"");
                }
                now_key = argv[i] + 1;
            }
            else{
                SYLAR_LOG_ERROR(g_logger) << "invalid arg index="<<i
                    << " val=" << argv[i];
                    return false;
            }
            
        }
        else{
            if(now_key){
                add(now_key,argv[i]);
                now_key = nullptr;
            }
            else{
                SYLAR_LOG_ERROR(g_logger) << "invalid arg index="<<i
                    << " val=" << argv[i];
                    return false;
            }
        }
    }
    if(now_key){
        add(now_key,"");
    }
    SYLAR_LOG_INFO(g_logger) <<"========== cur args ==========";
    for(auto & item : m_args){
        SYLAR_LOG_INFO(g_logger) << item.first <<" : "<<item.second;
    }
    return true;
}

void Env::add(const std::string & key,const std::string & val){
    RWMutexType::WriteLock lock(m_mutex);
    m_args[key] = val;
}


void Env::del(const std::string & key){
    RWMutexType::WriteLock lock(m_mutex);
    m_args.erase(key);
    
}

bool Env::has(const std::string & key){
    RWMutexType::ReadLock lock(m_mutex);
    auto it = m_args.find(key);
    return it != m_args.end();
}
std::string Env::get(const std::string & key,const std::string & default_value){
    RWMutexType::ReadLock lock(m_mutex);
    auto it = m_args.find(key);
    return it != m_args.end() ? it->second : default_value;
}

void Env::addHelp(const std::string & key,const std::string & desc){
    removeHelp(key);
    RWMutexType::WriteLock lock(m_mutex);
    m_helps.push_back(std::make_pair(key,desc));
}
void Env::removeHelp(const std::string & key){
    RWMutexType::WriteLock lock(m_mutex);
    for(auto it = m_helps.begin();it != m_helps.end();){
        if(it->first == key){
            it = m_helps.erase(it);
        }
        else{
            ++it;
        }
    }
}
void Env::printHelp(){
    RWMutexType::ReadLock lock(m_mutex);
    std::cout << "Usage: "<<m_program<<" [option]"<<std::endl;
    for(auto & i : m_helps){
        std::cout<<std::setw(5)<<"-"<<i.first<<" : "<<i.second<<std::endl;
    }
}

bool Env::setEnv(const std::string & key,const std::string & val){
    return !setenv(key.c_str(),val.c_str(),1);
}
std::string Env::getEnv(const std::string & key,const std::string & default_value){
    const char * v = getenv(key.c_str());
    if(v == nullptr){
        return default_value;
    }
    return v;
}

std::string Env::getAbsolutePath(const std::string & path)const{
    if(path.empty()){
        return "/";
    }
    if(path[0] == '/'){
        return path;
    }
    return m_cwd + path;
}

std::string Env::getConfigPath(){
    return getAbsolutePath(get("c","conf"));
}

} // namespace sylar