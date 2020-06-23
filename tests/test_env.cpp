/*
 * @Descripttion: 
 * @version: 
 * @Author: zsj
 * @Date: 2020-06-22 09:19:12
 * @LastEditors: zsj
 * @LastEditTime: 2020-06-22 10:44:20
 */ 
#include"../sylar/env.h"
#include<fstream>
#include<unistd.h>
#include<string>

struct A{
    A(){
        std::ifstream ifs("/proc/"+std::to_string(getpid())+"/cmdline",std::ios::binary);
        std::string content;
        content.resize(4096);

        ifs.read(&content[0],content.size());
        content.resize(ifs.gcount());
        std::cout<<content<<std::endl;
    }
};

A a;

int main(int argc, char *argv[])
{

    sylar::EnvMgr::GetInstance()->addHelp("s","start with the terminal");
    sylar::EnvMgr::GetInstance()->addHelp("d","run as daemon");
    sylar::EnvMgr::GetInstance()->addHelp("p","print help");
    if(sylar::EnvMgr::GetInstance()->init(argc,argv) == false){
        sylar::EnvMgr::GetInstance()->printHelp();
        return 0;
    }

    std::cout<<"exe="<<sylar::EnvMgr::GetInstance()->getExe() << std::endl;
    std::cout<<"cwd="<<sylar::EnvMgr::GetInstance()->getCwd() << std::endl;

    std::cout<<"path="<<sylar::EnvMgr::GetInstance()->getEnv("PATH","xxx") << std::endl;

    if(sylar::EnvMgr::GetInstance()->has("p")){
        sylar::EnvMgr::GetInstance()->printHelp();
    }
    return 0;
}
