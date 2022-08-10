#ifndef _JKYI_ENV_H_
#define _JKYI_ENV_H_

#include"JKYi/singleton.h"
#include"JKYi/mutex.h"

#include<map>
#include<vector>

namespace JKYi{

//用于对进行参数解析和环境变量的设置
class Env{
public:
    typedef RWMutex RWMutexType;

    //用来进行解析的函数
     bool init(int argc,char ** argv);

     void add(const std::string& key,const std::string& val);
     bool has(const std::string& key);
     void del(const std::string& key); 
     std::string get(const  std::string& key,const std::string& default_val = "");

     void addHelp(const std::string& key,const std::string& val);
     void removeHelp(const std::string& key);
     void printHelp();

     const std::string& getExe()const { return m_exe; }
     const std::string& getCwd()const { return m_cwd; }

     bool setEnv(const std::string& key,const std::string& val);
     std::string getEnv(const std::string& key,const std::string& default_val = "");

     //根据传入的path返回对应的绝对路径
     std::string getAbsolutePath(const std::string& path)const;

     std::string getAbsoluteWorkPath(const std::string& path)const;

     std::string getConfigPath();
private:
    RWMutexType m_mutex;
    //用来存储输入的命令行参数
    std::map<std::string,std::string> m_args;
    //存储参数的信息
    std::vector<std::pair<std::string,std::string>> m_helps;
    //启动程序的名称
    std::string m_program;
    //启动的可执行文件的绝对路径
    std::string m_exe;
    std::string m_cwd;
};

typedef JKYi::Singleton<Env> EnvMgr;
}

#endif
