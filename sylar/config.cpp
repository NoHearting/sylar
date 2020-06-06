/*
 * @Descripttion: 
 * @version: 
 * @Author: zsj
 * @Date: 2020-06-06 14:23:07
 * @LastEditors: zsj
 * @LastEditTime: 2020-06-06 22:49:08
 */ 
#include"config.h"

namespace sylar{


Config::ConfigVarMap Config::s_datas;

static void ListAllMember(const std::string & prefix,
                          const YAML::Node & node,
                          std::list<std::pair<std::string,const YAML::Node> > & output){
    if(prefix.find_first_not_of("abcdefghijklmnopqrstuvwxyz._0123456789") != std::string::npos){
            SYLAR_LOG_ERROR(SYLAR_LOG_ROOT())<<"Config invalid name:"<<prefix<<":"<<node;
            return;
    }
    output.push_back(std::make_pair(prefix,node));
    if(node.IsMap()){
        for(auto & iter : node){
            ListAllMember((prefix.empty() ? iter.first.Scalar():(prefix + "." + iter.first.Scalar())),iter.second,output);
        }
    }
}

void Config::LoadFromYaml(const YAML::Node & root)
{   
    std::list<std::pair<std::string,const YAML::Node> > all_nodes;
    
    ListAllMember("",root,all_nodes);
    for(auto & item : all_nodes){
        std::string key = item.first;
        if(key.empty()){
            continue;
        }
        std::transform(key.begin(),key.end(),key.begin(),::tolower);
        ConfigVarBase::ptr var = LookupBase(key);  //查找配置是否存在，有则覆盖，无则添加
        if(var){
            if(item.second.IsScalar()){
                var->fromString(item.second.Scalar());
            }
            else{
                std::stringstream ss;
                ss << item.second;
                var->fromString(ss.str());
            }
        }
    }
}

ConfigVarBase::ptr Config::LookupBase(const std::string & name)
{
    auto it = s_datas.find(name);
    if(it != s_datas.end()){
        return s_datas[name];
    }
    return nullptr;
}

}