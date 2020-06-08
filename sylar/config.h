/*
 * @Descripttion: 
 * - 定义配置系统，其主要组件有：
 *      - 配置变量（ConfigVar），代表一个配置量
 *      - 配置器（Config），记录了所有配置变量
 *      - 转换器（LexicalCast），及其对不同数据结构的偏特化版本
 * - 配置系统主要工作逻辑：
 *      - 可以利用配置器（Config）进行一些默认/约定配置
 *      - 然后使用配置器（Config）进行配置文件的读取，读取所有的配置变量（ConfigVar）
 *      - 在设置配置变量的时候会触发转化器（LexicalCast），将配置文件中的字符串序列化为当前配置变量的类型
 *      - 当配置文件配置和默认配置不一样时，会触发添加的监听事件，对配置进行修改
 * @version: 
 * @Author: zsj
 * @Date: 2020-06-05 20:06:46
 * @LastEditors: zsj
 * @LastEditTime: 2020-06-08 10:33:24
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
#include<functional>
#include<yaml-cpp/yaml.h>

#include"log.h"
#include"util.h"




namespace sylar{

/**
 * @brief 配置变量基类，定义了基础配置变量的名字和描述，
 *      - 提供配置变量字符串到数据结构的序列化和反序列化接口
 */ 
class ConfigVarBase{
public:
    typedef std::shared_ptr<ConfigVarBase> ptr;
    ConfigVarBase(const std::string & name,const std::string & description = "")
        :m_name(name),m_description(description)
    {
        //配置变量名称大小写不敏感
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
//--  * 定义了一系列的转化器类，用于配置变量字符串和内部数据结构的序列化和反序列化
//--  * 定义了一个模板类LexicalCast，将F类型转化为T类型，但只能用于普通对象的转换
//--  * 添加定义了不同数据结构的转化器——对于基本转化器的偏特化，支持偏特化的数据结构有：
//--        + vector
//--        + list
//--        + set
//--        + unordered_set
//--        + map
//--        + unordered_map
//--  * 支持自定义转化器用于序列化自定义类，但是必须编写自己的偏特化转化器，然后重载operator()
//--    * 使用的operator()进行转换
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
 * @brief 配置变量实现类，继承于ConfigVarBase类
 *  - 含有三个模板参数：
 *      - T：配置变量的类型
 *      - FromStr：转换器，从配置文件中的字符串转化为当前配置类型T
 *      - ToStr: 转换器，从当前配置类型转化为配置文件中的字符串
 */ 
template<class T,class FromStr = LexicalCast<std::string,T>
                ,class ToStr = LexicalCast<T,std::string>>
class ConfigVar : public ConfigVarBase{
public:
    typedef std::shared_ptr<ConfigVar> ptr;
    typedef std::function<void (const T & old_value,const T & new_value)> on_change_cb;

    ConfigVar(const std::string & name
            ,const T & default_value
            ,const std::string&description = "")
        :ConfigVarBase(name,description),m_val(default_value){

    }

    /**
     * @brief 将配置变量的值转化为yaml格式字符串并且返回
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
     * @brief 设置配置变量的值，该值由val（yaml字符串）转换而来
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

    /**
     * @brief 设置配置变量的值，
     *      - 如果检测到值有变化则调用当前配置变量上注册的回调函数进行处理
     */ 
    void setValue(const T & val){
        if(val == m_val){
            return;
        }
        for(auto & item : m_cbs){
            item.second(m_val,val);
        }
        m_val = val;
    } 

    std::string getTypeName()const override{return typeid(T).name();}

    void addListener(uint64_t key,on_change_cb cb){
        m_cbs[key] = cb;
    }

    void delListener(uint64_t key){
        m_cbs.erase(m_cbs.find(key));
    }

    on_change_cb getListener(uint64_t key){
        auto it = m_cbs.find(key);
        return it = m_cbs.end() ? nullptr : it->second;
    }

    void clearListener(){
        m_cbs.clear();
    }
private:
    T m_val;  //配置变量的值
    std::map<uint64_t,on_change_cb> m_cbs;  //变更回调函数组，用于监听配置变量是否有变化
};


/**
 * @brief 配置类
 *  - 
 */ 
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
        
        auto it = GetDatas().find(name);
        if(it != GetDatas().end()){
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
        GetDatas()[name] = v;
        return v;
    }

    /**
     * @brief 查找当前容器中有无此name的配置变量，有就返回，没有返回nullptr
     */ 
    template<class T>
    static typename ConfigVar<T>::ptr Lookup(const std::string & name){
        auto it = GetDatas().find(name);
        if(it == GetDatas().end()){
            return nullptr;
        }
        return std::dynamic_pointer_cast<ConfigVar<T>>(it->second);
    }

    /**
     * @brief 从配置文件加载配置,
     *  - 读取配置文件所有的配置到一个数据结构，
     *  - 然后判断配置文件中配置和当前约定配置是否有不同，有则更新约定配置
     */ 
    static void LoadFromYaml(const YAML::Node & root);

    /**
     * @brief 查找当前命名的项，有则返回
     */ 
    static ConfigVarBase::ptr LookupBase(const std::string & name);
private:

    

    /**
     * @brief 获取当前系统中所有的配置项
     *  - 所有的配置项容器为一个静态局部变量，使用本函数获取是为了初始化早于使用这个容器的方法
     */ 
    static ConfigVarMap & GetDatas(){
        static ConfigVarMap s_datas;
        return s_datas;
    }
};

}



#endif