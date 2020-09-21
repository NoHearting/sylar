/*
 * @Descripttion: 
 * @version: 
 * @Author: zsj
 * @Date: 2020-07-01 11:16:30
 * @LastEditors: zsj
 * @LastEditTime: 2020-07-05 19:47:56
 */ 
#include "module.h"
#include "config.h"
#include "env.h"
#include "library.h"
#include "util.h"



namespace sylar
{

static sylar::Logger::ptr g_logger = SYLAR_LOG_NAME("system");

static ConfigVar<std::string>::ptr g_module_path 
    = Config::Lookup("module.path",std::string("module"),"module path");

Module::Module(const std::string & name
              ,const std::string & version
              ,const std::string & filename)
    :m_name(name)
    ,m_version(version)
    ,m_filename(filename)
    ,m_id(name + "/" + version){

}

void Module::onBeforeArgsParse(int argc,char ** argv){
    
}

void Module::onAfterArgsParse(int agrc,char ** argv){
    
}

bool Module::onLoad(){
    SYLAR_LOG_INFO(g_logger) << "Top class onLoad";
    return true;
}

bool Module::onUnLoad(){
    SYLAR_LOG_INFO(g_logger) << "Top class onUnLoad";
    return true;
}

bool Module::onConnect(Stream::ptr stream){
    return true;
}

bool Module::onDisconnect(Stream::ptr stream){
    return true;
}

bool Module::onServerReady(){
    SYLAR_LOG_INFO(g_logger) << "Top class onServerReady";
    return true;
}

bool Module::onServerUp(){
    SYLAR_LOG_INFO(g_logger) << "Top class onServerUp";
    return true;
}

std::string Module::statusString(){
    return "";
}

ModuleManager::ModuleManager(){
    
}

Module::ptr ModuleManager::get(const std::string & name){
    RWMutexType::ReadLock lock(m_mutex);
    auto it = m_modules.find(name);
    return it == m_modules.end() ? nullptr : it->second;
}

void ModuleManager::add(Module::ptr m){

    //id = name + '/' + version
    //添加之前需要先删除
    del(m->getId());
    RWMutexType::WriteLock lock(m_mutex);
    m_modules[m->getId()] = m;
}

void ModuleManager::del(const std::string & name){
    Module::ptr module;
    RWMutexType::WriteLock lock(m_mutex);
    auto it = m_modules.find(name);
    if(it == m_modules.end()){
        return;
    }
    module = it->second;
    m_modules.erase(it);
    lock.unlock();

    //调用模块卸载函数
    module->onUnLoad();
}

void ModuleManager::delAll(){
    RWMutexType::ReadLock lock(m_mutex);
    auto tmp = m_modules;
    lock.unlock();

    for(auto & item : tmp){
        del(item.first);
    }
}


void ModuleManager::init(){
    auto path = EnvMgr::GetInstance()->getAbsolutePath(g_module_path->getValue());
    
    SYLAR_LOG_DEBUG(g_logger) << "module file path: " << g_module_path->getValue();
    std::vector<std::string> files;
    FSUtil::listAllFile(files,path,".so");
    SYLAR_LOG_DEBUG(g_logger) << "========= List File ============";
    for(auto & item : files){
        SYLAR_LOG_DEBUG(g_logger) << item;
    }
    std::sort(files.begin(),files.end());
    for(auto & i:files){
        initModule(i);
    }
}


void ModuleManager::onConnect(Stream:: ptr stream){
    std::vector<Module::ptr> ms;
    listAll(ms);

    for(auto & m:ms){
        m->onConnect(stream);
    }
}

void ModuleManager::onDisconnect(Stream::ptr stream){
    std::vector<Module::ptr> ms;
    listAll(ms);

    for(auto & m : ms){
        m->onDisconnect(stream);
    }
}

void ModuleManager::listAll(std::vector<Module::ptr> & ms){
    RWMutexType::ReadLock lock(m_mutex);
    for(auto & i : m_modules){
        ms.push_back(i.second);
    } 
}

void ModuleManager::initModule(const std::string & path){
    Module::ptr m = Library::GetModule(path);
    if(m){
        add(m);
    }
}

} // namespace sylar
