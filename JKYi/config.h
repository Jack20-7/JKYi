#ifndef _JKYI_CONFIG_H_
#define _JKYI_CONFIG_H_

#include<memory>
#include<sstream>
#include<string>
#include<boost/lexical_cast.hpp>
#include"log.h"
#include <vector>
#include <list>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <typeinfo>

#include"thread.h"
#include "log.h"
#include "util.h"

namespace JKYi{

//基类
class ConfigVarBase{
public:
   typedef std::shared_ptr<ConfigVarBase> ptr;
   ConfigVarBase(const std::string&name,const std::string&description="")
   :m_name(name),m_description(description){
      //这里的话，默认将名字全部转化为小写
      std::transform(m_name.begin(),m_name.end(),m_name.begin(),::tolower);
   }
   virtual ~ConfigVarBase(){};//因为要作为基类
   //
   const std::string& getName()const {return m_name;}
   const std::string& getDescription()const {return m_description;}
   //纯虚函数
   virtual std::string toString()=0;
   virtual bool fromString(const std::string&val)=0;
   virtual std::string getType()const=0;
private:
   std::string m_name;
   std::string m_description;
};
//定义一个用来转型的基本类型
//从F转到T
template<class F,class T>
class LexicalCast{
public:
   T operator()(const F&f){
      return boost::lexical_cast<T>(f);
   }
};
//太经典了，如果要将其他类型转化为string的话，只需要先将该类型的对象读到stringstream里面取，再返回stringstream里面的str()就可以了
//为了支持复杂类型的转化，这里对LexicalCast进行偏特化
//------------------------------------------------对vector的支持
//string->vector<T>
template<class T>
class LexicalCast<std::string,std::vector<T>>{
public:
   std::vector<T> operator()(const std::string&v){
       YAML::Node node=YAML::Load(v);
       typename std::vector<T> vec;
       std::stringstream ss;
       for(size_t i=0;i<node.size();++i){
          ss.str("");
          ss<<node[i];
          vec.push_back(LexicalCast<std::string,T>()(ss.str()));
       }
       return vec;
   }
};
//
//vector<T>->string
template<class T>
class LexicalCast<std::vector<T>,std::string>{
public:
   std::string operator()(std::vector<T>&v){
      YAML::Node node;
      for(auto&i:v){
         node.push_back(YAML::Load(LexicalCast<T,std::string>()(i)));
      }
      std::stringstream ss;
      ss<<node;
      return ss.str();
   }
};

//--------------------------------------------下面是对list容器的偏特化
template<class T>
class LexicalCast<std::string,std::list<T>>{
public:
   std::list<T> operator()(const std::string&v){
       YAML::Node node=YAML::Load(v);
       typename std::list<T> vec;
       std::stringstream ss;
       for(size_t i=0;i<node.size();++i){
          ss.str("");
          ss<<node[i];
          vec.push_back(LexicalCast<std::string,T>()(ss.str()));
       }
       return vec;
   }
};
//
//list<T>->string
template<class T>
class LexicalCast<std::list<T>,std::string>{
public:
   std::string operator()(std::list<T>&v){
      YAML::Node node;
      for(auto&i:v){
         node.push_back(YAML::Load(LexicalCast<T,std::string>()(i)));
      }
      std::stringstream ss;
      ss<<node;
      return ss.str();
   }
};
//                                              下面是对set容器的支持
template<class T>
class LexicalCast<std::string,std::set<T>>{
public:
   std::set<T> operator()(const std::string&v){
       YAML::Node node=YAML::Load(v);
       typename std::set<T> vec;
       std::stringstream ss;
       for(size_t i=0;i<node.size();++i){
          ss.str("");
          ss<<node[i];
          vec.insert(LexicalCast<std::string,T>()(ss.str()));
       }
       return vec;
   }
};
//
//set<T>->string
template<class T>
class LexicalCast<std::set<T>,std::string>{
public:
   std::string operator()(std::set<T>&v){
      YAML::Node node;
      for(auto&i:v){
         node.push_back(YAML::Load(LexicalCast<T,std::string>()(i)));
      }
      std::stringstream ss;
      ss<<node;
      return ss.str();
   }
};
//                              下面是对unordered_set容器的支持
template<class T>
class LexicalCast<std::string,std::unordered_set<T>>{
public:
   std::unordered_set<T> operator()(const std::string&v){
       YAML::Node node=YAML::Load(v);
       typename std::unordered_set<T> vec;
       std::stringstream ss;
       for(size_t i=0;i<node.size();++i){
          ss.str("");
          ss<<node[i];
          vec.insert(LexicalCast<std::string,T>()(ss.str()));
       }
       return vec;
   }
};
//
//map<T>->string
template<class T>
class LexicalCast<std::unordered_set<T>,std::string>{
public:
   std::string operator()(std::unordered_set<T>&v){
      YAML::Node node;
      for(auto&i:v){
         node.push_back(YAML::Load(LexicalCast<T,std::string>()(i)));
      }
      std::stringstream ss;
      ss<<node;
      return ss.str();
   }
};
//                                                       下面是map容器的支持 
//string->map<std::string,T>
template<class T>            
class LexicalCast<std::string,std::map<std::string,T>>{
public:
   std::map<std::string,T> operator()(const std::string&v){
       YAML::Node node=YAML::Load(v);
       typename std::map<std::string,T> vec;
       std::stringstream ss;
       for(auto it=node.begin();it!=node.end();++it){
          ss.str("");
          ss<<it->second;
          vec.insert({it->first.Scalar(),LexicalCast<std::string,T>()(ss.str())});
       }
       
       return vec;
   }
};
//
//map<std::string,T>->string
template<class T>
class LexicalCast<std::map<std::string, T>, std::string> {
public:
    std::string operator()(const std::map<std::string, T>& v) {
        YAML::Node node(YAML::NodeType::Map);
        for(auto& i : v) {
            node[i.first] = YAML::Load(LexicalCast<T, std::string>()(i.second));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};
//                                                        下面是unordered_map容器的支持
//string->unorder_map<std::string,T>
template<class T>            
class LexicalCast<std::string,std::unordered_map<std::string,T>>{
public:
   std::unordered_map<std::string,T> operator()(const std::string&v){
       YAML::Node node=YAML::Load(v);
       typename std::unordered_map<std::string,T> vec;
       std::stringstream ss;
       for(auto it=node.begin();it!=node.end();++it){
          ss.str("");
          ss<<it->second;
          vec.insert({it->first.Scalar(),LexicalCast<std::string,T>()(ss.str())});
       }
       
       return vec;
   }
};
//
//unordered_map<std::string,T>->string
template<class T>
class LexicalCast<std::unordered_map<std::string, T>, std::string> {
public:
    std::string operator()(const std::unordered_map<std::string, T>& v) {
        YAML::Node node(YAML::NodeType::Map);
        for(auto& i : v) {
            node[i.first] = YAML::Load(LexicalCast<T, std::string>()(i.second));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

//子类
template<class T,class FromStr=LexicalCast<std::string,T>,class ToStr=LexicalCast<T,std::string>>
class ConfigVar:public ConfigVarBase{
public:
   typedef std::shared_ptr<ConfigVar<T>> ptr;
   typedef std::function<void (const  T&oldValue,const T&newValue)> on_change_cb;
   typedef RWMutex RWMutexType;

   ConfigVar(const std::string&name,const T&default_value,const std::string&description="")
   :ConfigVarBase(name,description),m_val(default_value){}
   //该函数的作用就是用来将m_val转化为string类型
   std::string toString()override{
       try{
         //return boost::lexical_cast<std::string>(m_val);
		 RWMutex::ReadLock lock(m_mutex);
         return ToStr()(m_val);
       }catch(std::exception&e){
         JKYI_LOG_ERROR(JKYI_LOG_ROOT())<<"ConfigVar::toString exception"
         <<e.what()<<"convert:"<<typeid(m_val).name()<<" to string";
       }
       return "";

   }
   //将m_val转化为T类型
   bool fromString(const std::string&val)override{
      try{
         //m_val=boost::lexical_cast<T>(val);
         setValue(FromStr()(val));
      }catch(std::exception& e){
         JKYI_LOG_ERROR(JKYI_LOG_ROOT())<<"ConfigVar::FromString exception "
         <<e.what()<<" convert: string to "<<typeid(m_val).name();
      }
      return false;
   }
   //
   const T getValue(){
         RWMutexType::ReadLock lock(m_mutex);
         return m_val;
   }
   void setValue(const T&t){
	{
      RWMutex::ReadLock lock(m_mutex);	
      if(m_val==t){
         return ;
      }
      //不相等的话，代表发生的值的变化，所以需要依次调用每一个回调函数
      for(auto&i : m_cbs){
         i.second(m_val,t);
      }
	}
	  RWMutexType::WriteLock lock(m_mutex);
      m_val=t;
   }
   std::string getType()const override { return typeid(T).name();}
   //回调函数相关
   uint64_t addListener(on_change_cb cb){
	  static uint64_t s_fun_id=0;
	  RWMutexType::WriteLock lock(m_mutex);
	  ++s_fun_id;
      m_cbs[s_fun_id]=cb;
	  return s_fun_id;
   }
   void delListener(uint64_t key){
	  RWMutexType::WriteLock lock(m_mutex);
      m_cbs.erase(key);
   }
   on_change_cb getListener(uint64_t key){
	   RWMutexType::ReadLock lock(m_mutex);
      auto it=m_cbs.find(key);
      return it==m_cbs.end()?nullptr:it->second;
   }
   void clearListener(){
	  RWMutexType::WriteLock lock(m_mutex);
      m_cbs.clear();
      return ;
   }
private:
   T m_val;
   //用来实现事件检测机制，也就是当发生了变更时需要调度的回调函数
   std::unordered_map<uint64_t,on_change_cb>m_cbs;
   //用读写锁效率会比较高一点
   RWMutexType m_mutex;
};
//管理类
class Config{
public:
    typedef std::map<std::string,ConfigVarBase::ptr> ConfigVarMap;
	typedef RWMutex RWMutexType;
    //查找，不存在就创建新的
    template<class T>
    static typename ConfigVar<T>::ptr Lookup(const std::string&name,
    const T&default_value,const std::string&description=""){
        //这里需要特别处理那些名称相同但是类型不同的configvar
		RWMutexType::WriteLock lock(getMutex());
        auto it=getDatas().find(name);
        if(it!=getDatas().end()){
           //如果存在的话，需要判断是否是类型相同的
           auto tmp=std::dynamic_pointer_cast<ConfigVar<T>>(it->second);
           if(tmp){//如果能够转型成功的话，那么就表示是同一种类型
            JKYI_LOG_INFO(JKYI_LOG_ROOT())<<"Lookup name="<<name<<"exists";
            return tmp;
           }else{
              //转型失败，代表不是同一种类型
              JKYI_LOG_ERROR(JKYI_LOG_ROOT())<<"Lookup name="<<name<<"exists but type not"
              <<typeid(T).name()<<"real_type="<<it->second->getType();
              return nullptr;
           }
        }
        //这里还需要判断名称是否合法
        if(name.find_first_not_of("abcdefghijklmnopqrstuvwxyz._012345678")!=std::string::npos){
            JKYI_LOG_ERROR(JKYI_LOG_ROOT())<<"Lookup name invalid:"<<name;
            throw std::invalid_argument(name);
        }
        //不存在就需要进行创建
        typename ConfigVar<T>::ptr v(new ConfigVar<T>(name,default_value,description));
        getDatas()[name]=v;
        
        return v;
    }
    //
    template<class T>
    static typename ConfigVar<T>::ptr Lookup(const std::string&name){
		RWMutexType::ReadLock lock(getMutex());
        auto it=getDatas().find(name);
        if(it==getDatas().end()){
           return nullptr;
        }
        return std::dynamic_pointer_cast<ConfigVar<T>>(it->second);
    }

    //
    static void LoadFromYaml(const YAML::Node&node);

    //将path目录下的配置文件进行加载  
    static void LoadFromConfDir(const std::string& path,bool force = false);

    static ConfigVarBase::ptr LookupBase(const std::string&name);
	//该函数提供给用户用于获取当前系统中的配置信息
	static void Visit(std::function<void (ConfigVarBase::ptr)>cb); 
private: 
    //这里为了解决全局变量初始化顺序不可预知的问题，这里将成员放到static成员函数里面去
    static ConfigVarMap& getDatas(){
       static ConfigVarMap m_datas;
        return m_datas;
    }
	//这里还是为了避免全局全局变量和静态变量初始化顺序的问题，将锁变成static局部变量
	static RWMutexType& getMutex(){
	   static  RWMutexType m_mutex;
	   return m_mutex;
	}
};
}
#endif
