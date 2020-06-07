/*
 * @Descripttion: 
 * @version: 
 * @Author: zsj
 * @Date: 2020-06-06 14:18:03
 * @LastEditors: zsj
 * @LastEditTime: 2020-06-07 20:55:20
 */ 
#include<yaml-cpp/yaml.h>
#include<iostream>
#include"../sylar/config.h"
#include"../sylar/log.h"

#if 0

sylar::ConfigVar<int>::ptr g_int_value_config = 
    sylar::Config::Lookup("system.port",(int)8080,"system port");

sylar::ConfigVar<float>::ptr g_int_valuex_config = 
    sylar::Config::Lookup("system.port",(float)8080,"system port");


sylar::ConfigVar<std::vector<int>>::ptr g_int_vec_value_config = 
    sylar::Config::Lookup("system.int_vec",std::vector<int>{1,2},"system int vec");


sylar::ConfigVar<std::list<int>>::ptr g_int_list_value_config = 
    sylar::Config::Lookup("system.int_list",std::list<int>{1,2},"system int list");

sylar::ConfigVar<std::set<int>>::ptr g_int_set_value_config = 
    sylar::Config::Lookup("system.int_set",std::set<int>{1,2},"system int set");

sylar::ConfigVar<std::unordered_set<int>>::ptr g_int_uset_value_config = 
    sylar::Config::Lookup("system.int_uset",std::unordered_set<int>{1,2},"system int uset");

sylar::ConfigVar<std::map<std::string,int>>::ptr g_int_str_map_value_config = 
    sylar::Config::Lookup("system.str_int_map",std::map<std::string,int>{{"k",2}},"system int map");

sylar::ConfigVar<std::unordered_map<std::string,int>>::ptr g_int_str_umap_value_config = 
    sylar::Config::Lookup("system.str_int_umap",std::unordered_map<std::string,int>{{"k",2}},"system int umap");


void print_yaml(const YAML::Node & node,int level){
    if(node.IsScalar()){
        SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << std::string(level*4,' ')
            << node.Scalar() << " - "<<node.Tag() << " - "<<level;
    }
    else if(node.IsNull()){
        SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << std::string(level*4,' ')
             << "NULL - "<< node.Tag() << " - "<<level;
    }
    else if(node.IsMap()){
        for(auto & iter : node){
            SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << std::string(level*4,' ')
                <<iter.first<<" - "<<iter.second.Tag() << " - "<<level;
            print_yaml(iter.second,level+1);
        }
    }
    else if(node.IsSequence()){
        for(size_t i = 0;i<node.size();++i){
            SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << std::string(level*4,' ')
                << i << " - "<<node[i].Type()<<" - "<<level;
                print_yaml(node[i],level+1);
        }
    }
    

}

void test_yaml(){
    YAML::Node root = YAML::LoadFile("/home/zsj/workspace/sylar/bin/conf/test.yml");
    print_yaml(root,0);
    // SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << root;
}

void test_config(){
    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) <<"before"<< g_int_value_config->getValue();
    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) <<"before"<< g_int_value_config->toString();

#define XX(g_var,name,prefix) \
    { \
        auto v = g_var->getValue(); \
        for(auto & i : v){ \
            SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << #prefix " " #name " : " << i; \
        } \
        SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << #prefix " " #name " yaml: " << g_var->toString(); \
    } 


#define XX_M(g_var,name,prefix) \
    { \
        auto v = g_var->getValue(); \
        for(auto & i : v){ \
            SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << #prefix " " #name " : {"  \
                << i.first << " - "<<i.second<<"}"; \
        } \
        SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << #prefix " " #name " yaml: " << g_var->toString(); \
    } 
    
    XX(g_int_vec_value_config,int_vec,before);
    XX(g_int_list_value_config,int_list,before);
    XX(g_int_set_value_config,int_set,before);
    XX(g_int_uset_value_config,int_uset,before);
    XX_M(g_int_str_map_value_config,str_int_map,before);
    XX_M(g_int_str_umap_value_config,str_int_umap,before);






    YAML::Node root = YAML::LoadFile("/home/zsj/workspace/sylar/bin/conf/test.yml");
    sylar::Config::LoadFromYaml(root);

    std::cout<<std::endl;

    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) <<"after"<< g_int_value_config->getValue();
    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) <<"after"<< g_int_value_config->toString();

    XX(g_int_vec_value_config,int_vec,after);
    XX(g_int_list_value_config,int_list,after);
    XX(g_int_set_value_config,int_set,after);
    XX(g_int_uset_value_config,int_uset,after);
    XX_M(g_int_str_map_value_config,str_int_map,after);
    XX_M(g_int_str_umap_value_config,str_int_umap,after);

}

#endif

class Person{
public:
    Person(){}
    Person(const std::string & name,int age,int sex):
        m_name(name),m_age(age),m_sex(sex){}

    std::string m_name = "default";
    int m_age = 0;
    bool m_sex = 0;

    bool operator==(const Person & oth) const{
        return m_name == oth.m_name &&
               m_age == oth.m_age &&
               m_sex == oth.m_sex;
    }

    std::string toString()const{
        std::stringstream ss;
        ss<<"[Person name="<<m_name
          <<" age="<<m_age
          <<" sex="<<m_sex
          <<"]"<<std::endl;
        return ss.str();
    }
private:
    
};


namespace sylar{
    
template<>
class LexicalCast<std::string,Person>{
public:
    Person operator()(const std::string & v){
        YAML::Node node = YAML::Load(v);
        Person p;
        try{
            p.m_name = node["name"].as<std::string>();
            p.m_age = node["age"].as<int>();
            p.m_sex = node["sex"].as<bool>();
        }catch(std::exception & e){
            SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "yaml data  format error!";
            throw e;
        }
        
        return p;
    }
};

// person to string
template<>
class LexicalCast<Person,std::string>{
public:
    std::string operator()(const Person & v){
        YAML::Node node;
        node["name"] = v.m_name;
        node["age"] = v.m_age;
        node["sex"] = v.m_sex;
        std::stringstream ss;
        ss<<node;
        return ss.str();
    }
};
}

sylar::ConfigVar<Person>::ptr g_person = 
    sylar::Config::Lookup("class.person",Person(),"class person");

sylar::ConfigVar<std::map<std::string,Person>>::ptr g_person_map = 
    sylar::Config::Lookup("class.map",std::map<std::string,Person>(),"class person map");

sylar::ConfigVar<std::map<std::string,std::vector<Person>>>::ptr g_person_vec_map = 
    sylar::Config::Lookup("class.vec_map",std::map<std::string,std::vector<Person>>(),"class person vec_map");


void test_class(){
    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) <<" before "<< g_person->getValue().toString() << " - " <<g_person->toString();


#define XX_PM(g_var,name,prefix) \
    { \
        auto v = g_var->getValue(); \
        for(auto & i : v){ \
            SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << #prefix " " #name " : {"  \
                << i.first << " - "<<i.second.toString()<<"}"; \
        } \
        SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << #prefix " " #name " size: " << g_var->getValue().size(); \
    } 

    XX_PM(g_person_map,map,before);
    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) <<" before: " << g_person_vec_map->toString();

    g_person->addListener(10,[](const Person & old_value,const Person & new_value){
        SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "value change: "<< old_value.toString() <<" to "<<new_value.toString();
    });

    YAML::Node root = YAML::LoadFile("/home/zsj/workspace/sylar/bin/conf/test.yml");
    sylar::Config::LoadFromYaml(root);

    
    // sylar::Config::Lookup("class.person",Person("test",100,1),"class person");

    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) <<" after "<< g_person->getValue().toString() << " - " <<g_person->toString();
    XX_PM(g_person_map,map,after);
    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) <<" after: " << g_person_vec_map->toString();

}

void test_log(){
    SYLAR_LOG_INFO(SYLAR_LOG_NAME("system"))<<"hello config";
    // std::cout<<sylar::LogLevel::toString(SYLAR_LOG_NAME("system")->getLevel())<<std::endl;
    // std::cout<<sylar::LoggerMgr::GetInstance()->toYamlString()<<std::endl;
    YAML::Node root = YAML::LoadFile("/home/zsj/workspace/sylar/bin/conf/log.yml");
    sylar::Config::LoadFromYaml(root);
    // std::cout<<sylar::LoggerMgr::GetInstance()->toYamlString()<<std::endl;
    // std::cout<<sylar::LogLevel::toString(SYLAR_LOG_NAME("system")->getLevel())<<std::endl;
    SYLAR_LOG_INFO(SYLAR_LOG_NAME("system"))<<"hello config";
    // SYLAR_LOG_FATAL(SYLAR_LOG_NAME("system"))<<"hello config";
}

int main(int argc,char * argv[])
{
    

    // test_yaml();
    // test_config();
    // test_class();
    test_log();
}