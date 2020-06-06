/*
 * @Descripttion: 
 * @version: 
 * @Author: zsj
 * @Date: 2020-06-05 20:06:46
 * @LastEditors: zsj
 * @LastEditTime: 2020-06-06 23:38:26
 */ 
#ifndef SYLAR__CONFIG_H__
#define SYLAR__CONFIG_H__

#include<memory>
#include<sstream>
#include<boost/lexical_cast.hpp>
#include<map>
#include<unordered_map>
#include<set>
#include<unordered_set>
#include<vector>
#include<list>
#include<utility>
#include<yaml-cpp/yaml.h>

#include"log.h"
#include"util.h"




namespace sylar{

class ConfigVarBase{
public:
    typedef std::shared_ptr<ConfigVarBase> ptr;
    ConfigVarBase(const std::string & name,const std::string & description = "")
        :m_name(name),m_description(description)
    {
        std::transform(m_name.begin(),m_name.end(),m_name.begin(),::tolower);
    }
    virtual ~ConfigVarBase(){}

    const std::string getName()const {return m_name;}
    const std::string getDescription()const{return m_description;}
    void setName(const std::string name) {m_name = name;}
    void setDescription(const std::string description){m_description = description;}

    virtual std::string toString() = 0;
    virtual bool fromString(const std::string & val) = 0;
    virtual std::string getTypeName()const = 0;
protected:
    std::string m_name;             //名字
    std::string m_description;      //描述
};

//--------------------------------------------------------------------------------------------
//--------------------------------------     自定义转化类     ----------------------------------
//--------------------------------------------------------------------------------------------

/**
 * @brief 将F类型转化为T类型
 */ 
template<typename F,typename T>
class LexicalCast{
public:
    T operator()(const F & v){
        return boost::lexical_cast<T>(v);
    }
};

/**
 * @brief 一个偏特化版本，用于string向vector转化
 * 
 * template<typename F,typename T>
 * class LexicalCast{...}
 * 的一个偏特化版本，用于string向vector转化
 */ 
template<typename T>
class LexicalCast<std::string,std::vector<T>>{
public:
    std::vector<T> operator()(const std::string & v){
        YAML::Node node = YAML::Load(v);
        typename std::vector<T> vec;
        std::stringstream ss;
        for(size_t i = 0;i<node.size();i++){
            ss.str("");
            ss<<node[i];
            vec.push_back(LexicalCast<std::string,T>()(ss.str()));
        }
        return vec;
    }
};

// vector to string
template<typename T>
class LexicalCast<std::vector<T>,std::string>{
public:
    std::string operator()(const std::vector<T> & v){
        YAML::Node node;
        for(auto & item : v){
            node.push_back(YAML::Load(LexicalCast<T,std::string>()(item)));
        }
        std::stringstream ss;
        ss<<node;
        return ss.str();
    }
};


//string to list
template<typename T>
class LexicalCast<std::string,std::list<T>>{
public:
    std::list<T> operator()(const std::string & v){
        YAML::Node node = YAML::Load(v);
        typename std::list<T> vec;
        std::stringstream ss;
        for(size_t i = 0;i<node.size();i++){
            ss.str("");
            ss<<node[i];
            vec.push_back(LexicalCast<std::string,T>()(ss.str()));
        }
        return vec;
    }
};

//list to string
template<typename T>
class LexicalCast<std::list<T>,std::string>{
public:
    std::string operator()(const std::list<T> & v){
        YAML::Node node;
        for(auto & item : v){
            node.push_back(YAML::Load(LexicalCast<T,std::string>()(item)));
        }
        std::stringstream ss;
        ss<<node;
        return ss.str();
    }
};


// string to set
template<typename T>
class LexicalCast<std::string,std::set<T>>{
public:
    std::set<T> operator()(const std::string & v){
        YAML::Node node = YAML::Load(v);
        typename std::set<T> vec;
        std::stringstream ss;
        for(size_t i = 0;i<node.size();i++){
            ss.str("");
            ss<<node[i];
            vec.insert(LexicalCast<std::string,T>()(ss.str()));
        }
        return vec;
    }
};

// set to string
template<typename T>
class LexicalCast<std::set<T>,std::string>{
public:
    std::string operator()(const std::set<T> & v){
        YAML::Node node;
        for(auto & item : v){
            node.push_back(YAML::Load(LexicalCast<T,std::string>()(item)));
        }
        std::stringstream ss;
        ss<<node;
        return ss.str();
    }
};

// string to unordered_set
template<typename T>
class LexicalCast<std::string,std::unordered_set<T>>{
public:
    std::unordered_set<T> operator()(const std::string & v){
        YAML::Node node = YAML::Load(v);
        typename std::unordered_set<T> vec;
        std::stringstream ss;
        for(size_t i = 0;i<node.size();i++){
            ss.str("");
            ss<<node[i];
            vec.insert(LexicalCast<std::string,T>()(ss.str()));
        }
        return vec;
    }
};

// unordered_set to string
template<typename T>
class LexicalCast<std::unordered_set<T>,std::string>{
public:
    std::string operator()(const std::unordered_set<T> & v){
        YAML::Node node;
        for(auto & item : v){
            node.push_back(YAML::Load(LexicalCast<T,std::string>()(item)));
        }
        std::stringstream ss;
        ss<<node;
        return ss.str();
    }
};


// string to map
template<typename T>
class LexicalCast<std::string,std::map<std::string,T>>{
public:
    std::map<std::string,T> operator()(const std::string & v){
        YAML::Node node = YAML::Load(v);
        typename std::map<std::string,T> vec;
        std::stringstream ss;
        for(const auto & item : node){
            ss.str("");
            ss<<item.second;
            vec.insert(std::make_pair(item.first.Scalar(),LexicalCast<std::string,T>()(ss.str())));
        }
        return vec;
    }
};

// map to string
template<typename T>
class LexicalCast<std::map<std::string,T>,std::string>{
public:
    std::string operator()(const std::map<std::string,T> & v){
        YAML::Node node;
        for(auto & item : v){
            node[item.first] = YAML::Load(LexicalCast<T,std::string>()(item.second));
        }
        std::stringstream ss;
        ss<<node;
        return ss.str();
    }
};

// string to unordered_map
template<typename T>
class LexicalCast<std::string,std::unordered_map<std::string,T>>{
public:
    std::unordered_map<std::string,T> operator()(const std::string & v){
        YAML::Node node = YAML::Load(v);
        typename std::unordered_map<std::string,T> vec;
        std::stringstream ss;
        for(const auto & item : node){
            ss.str("");
            ss<<item.second;
            vec.insert(std::make_pair(item.first.Scalar(),LexicalCast<std::string,T>()(ss.str())));
        }
        return vec;
    }
};

// unordered_map to string
template<typename T>
class LexicalCast<std::unordered_map<std::string,T>,std::string>{
public:
    std::string operator()(const std::unordered_map<std::string,T> & v){
        YAML::Node node;
        for(auto & item : v){
            node[item.first] = YAML::Load(LexicalCast<T,std::string>()(item.second));
        }
        std::stringstream ss;
        ss<<node;
        return ss.str();
    }
};





//------------------------------------转化类定义完成---------------------------------
//--------------------------------------------------------------------------------



/**
 * @brief 配置变量实现类
 */ 
template<class T,class FromStr = LexicalCast<std::string,T>
                ,class ToStr = LexicalCast<T,std::string>>
class ConfigVar : public ConfigVarBase{
public:
    typedef std::shared_ptr<ConfigVar> ptr;
    ConfigVar(const std::string & name
            ,const T & default_value
            ,const std::string&description = "")
        :ConfigVarBase(name,description),m_val(default_value){

    }

    /**
     * @brief 将配置变量的值转化为字符串并且返回
     * @return 配置变量的值的字符串
     */ 
    std::string toString()override{
        try{
            // return boost::lexical_cast<std::string>(m_val);
            return ToStr()(m_val);
        }catch(std::exception & e){
            SYLAR_LOG_ERROR(SYLAR_LOG_ROOT())<<"ConfigVar::toString exception"
                <<e.what()<<" convert: "<<typeid(m_val).name()<<" to string";
        }
        return "";
    }

    /**
     * @brief 设置配置变量的值，该值由val转换而来
     * 
     * @param val 字符串的值，转换后用于设置配置变量的值
     * @return 成功返回true，异常返回false
     */ 
    bool fromString(const std::string & val) override{
        try{
            setValue(FromStr()(val));
            return true;
        }catch(std::exception & e){
            SYLAR_LOG_ERROR(SYLAR_LOG_ROOT())<<"ConfigVar::fromString exception  "
                <<e.what()<<" convert: string to "<<typeid(T).name();
            return false;
        }
    }

    
    const T getValue()const {return m_val;}
    void setValue(const T & val){m_val = val;} 

    std::string getTypeName()const override{return typeid(T).name();}
private:
    T m_val;  //配置变量的值
};

class Config{
public:
    typedef std::shared_ptr<Config> ptr;
    typedef std::map<std::string,ConfigVarBase::ptr> ConfigVarMap;


    /**
     * @brief 查找当前容器有无此配置变量，有就直接使用，没有就创建
     */ 
    template<class T>
    static typename ConfigVar<T>::ptr Lookup(const std::string & name,
            const T & default_value,const std::string & description = ""){
        
        auto it = s_datas.find(name);
        if(it != s_datas.end()){
            auto tmp = std::dynamic_pointer_cast<ConfigVar<T>>(it->second);
            if(tmp){
                SYLAR_LOG_INFO(SYLAR_LOG_ROOT())<< "Lookup name="<<name<<" exists";
                return tmp;
            }
            else{
                SYLAR_LOG_INFO(SYLAR_LOG_ROOT())<< "Lookup name=" << name <<" exists but type not "
                    << typeid(T).name() << " real_type=" << it->second->getTypeName()
                    <<" "<<it->second->toString();
                return nullptr;
            }
        }
    
        if(name.find_first_not_of("abcdefghijklmnopqrstuvwxyz._0123456789") != std::string::npos){
            SYLAR_LOG_ERROR(SYLAR_LOG_ROOT())<<"Lookup name invalid"<<name;
            throw std::invalid_argument(name);
        }
        typename ConfigVar<T>::ptr v(new ConfigVar<T>(name,default_value,description));
        s_datas[name] = v;
        return v;
    }

    /**
     * @brief 查找当前容器中有无此name的配置变量，有就返回，没有返回nullptr
     */ 
    template<class T>
    static typename ConfigVar<T>::ptr Lookup(const std::string & name){
        auto it = s_datas.find(name);
        if(it == s_datas.end()){
            return nullptr;
        }
        return std::dynamic_pointer_cast<ConfigVar<T>>(it->second);
    }

    static void LoadFromYaml(const YAML::Node & root);

    /**
     * @brief 查找当前命名的项，有则返回
     */ 
    static ConfigVarBase::ptr LookupBase(const std::string & name);
private:

    //当前系统中所有的配置项
    static ConfigVarMap s_datas;
};

}



#endif