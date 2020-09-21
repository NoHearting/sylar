/*
 * @Descripttion: 
 * @version: 
 * @Author: zsj
 * @Date: 2020-07-01 11:19:32
 * @LastEditors: zsj
 * @LastEditTime: 2020-07-05 19:40:09
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
        m_destory(module);
        int rt = dlclose(m_handle);  //! dlcose()函数
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
    /**
     * @brief dlopen打开一个动态链接库
     * 在dlopen的（）函数以指定模式打开指定的动态连接库文件，并返回一个句柄给调用进程。
     * 使用dlclose（）来卸载打开的库。
     * !RTLD_LAZY 暂缓决定，等有需要时再解出符号 
     * !RTLD_NOW 立即决定，返回前解除所有未决定的符号。
     * void *dlopen (const char *__file, int __mode) 
     * @param __file 文件名
     * @param __mode 标志
     * 
     * @return 成功返回库的引用，失败返回null
     */ 
    void * handle = dlopen(path.c_str(),RTLD_NOW);
    if(!handle){
        SYLAR_LOG_ERROR(g_logger) << "cannot load libaray path="
            << path << " error" << dlerror();
        return nullptr;
    }

    /**
     * @brief dlsym根据动态链接库操作句柄(pHandle)与符号(symbol),返回符号对应的地址。
     *  使用这个函数不但可以获取函数地址，也可以获取变量地址
     * void *dlsym (void *__restrict __handle,const char *__restrict __name) 
     * @param __handle dlopen打开动态链接库后返回的指针
     * @param __name 要求获取的函数的名称
     * 
     * @return 返回的函数的地址
     * 
     */ 

    //create 返回一个Module
    create_module create = (create_module)dlsym(handle,"CreateModule");
    if(!create){
        SYLAR_LOG_ERROR(g_logger) << "cannot load symbol CreateModule in" 
            << path << " error=" << dlerror();
        dlclose(handle);
        return nullptr;
    }

    //destory 删除一个Module
    destory_module destory = (destory_module)dlsym(handle,"DestoryModule");
    if(!destory){
        SYLAR_LOG_ERROR(g_logger) << "cannot load symbol DestoryModule in" 
            << path << " error=" << dlerror();
        dlclose(handle);
        return nullptr;
    }

    //module智能指针对象  设置对象西沟时的处理对象，当模块使用完时自动释放
    Module::ptr module(create(),ModuleCloser(handle,destory));
    module->setFilename(path);
    SYLAR_LOG_INFO(g_logger) << "load module name="  << module->getName()
        << " version=" << module->getVersion()
        << " path=" << module->getFilename()
        << " success";
    
    //加载一次配置文件
    Config::LoadFromConfDir(EnvMgr::GetInstance()->getConfigPath(),true);
    return module;
}
    
} // namespace sylar

