/*
 * @Descripttion: 
 * @version: 
 * @Author: zsj
 * @Date: 2020-07-01 11:19:32
 * @LastEditors: zsj
 * @LastEditTime: 2020-07-01 11:37:01
 */ 
#include "library.h"

#include <dlfcn.h>
#include "sylar/config.h"
#include "sylar/env.h"
#include "sylar/log.h"


namespace sylar
{

static  Logger::ptr g_logger = SYLAR_LOG_NAME("system");

typedef Module* (*create_module)();
typedef void (*destory_module)(Module *);

class ModuleCloser{
public:
    ModuleCloser(void * handle,destory_module d)
        :m_handle(handle)
        ,m_destory(d){
            
    }

    void operator()(Module * module){
        std::string name = module->getName();
        std::string version = module->getVersion();
        std::string path = module->getFilename();
        int rt = dlclose(module);  //! dlcose()函数
        if(rt){
            SYLAR_LOG_ERROR(g_logger) << "dlclose hanler fail handler="
                << m_handle << " name=" << name
                << " version=" << version
                << " path=" << path
                << " error=" << dlerror();
        }
        else{
            SYLAR_LOG_INFO(g_logger) << "destory module=" << name
                << " version=" << version
                << " path=" << path
                << " handle" << m_handle
                << " success";
        }
    }

private:
    void * m_handle;
    destory_module m_destory;
};

Module::ptr Library::GetModule(const std::string & path){
    void * handle = dlopen(path.c_str(),RTLD_NOW);
    if(!handle){
        SYLAR_LOG_ERROR(g_logger) << "cannot load libaray path="
            << path << " error" << dlerror();
        return nullptr;
    }

    create_module create = (create_module)dlsym(handle,"CreateModule");
    if(!create){
        SYLAR_LOG_ERROR(g_logger) << "cannot load symbol CreateModule in" 
            << path << " error=" << dlerror();
        dlclose(handle);
        return nullptr;
    }

    destory_module destory = (destory_module)dlsym(handle,"DestoryModule");
    if(!destory){
        SYLAR_LOG_ERROR(g_logger) << "cannot load symbol DestoryModule in" 
            << path << " error=" << dlerror();
        dlclose(handle);
        return nullptr;
    }

    Module::ptr module(create(),ModuleCloser(handle,destory));
    module->setFilename(path);
    SYLAR_LOG_INFO(g_logger) << "load module name="  << module->getName()
        << " version=" << module->getVersion()
        << " path=" << module->getFilename()
        << " success";
    
    Config::LoadFromConfDir(EnvMgr::GetInstance()->getConfigPath(),true);
    return module;
}
    
} // namespace sylar

