#include"env.h"
#include"log.h"
#include"config.h"

#include<string.h>
#include<iostream>
#include<iomanip>
#include<unistd.h>
#include<stdlib.h>

namespace JKYi{

static Logger::ptr g_logger = JKYI_LOG_NAME("system");

bool Env::init(int argc,char ** argv){
    char link[1024] = {0};
    char path[1024] = {0};
    //原本的路径 
    sprintf(link,"/proc/%d/exe",getpid());
    //根据原本的路径对真实的路径进行写入
    readlink(link,path,sizeof(path));
    m_exe = path;
    auto pos = m_exe.find_last_of("/");
    m_cwd = m_exe.substr(0,pos) + "/";

    m_program = argv[0];
    const char * now_key = nullptr;
    for(int i = 1;i < argc;++i){
        if(argv[i][0] == '-'){
            //如果是key的话
            if(strlen(argv[i]) > 1){
                if(now_key){
                   //如果上一个key都还没有与之对于的value的话
                   add(now_key,"");
                }
                now_key = argv[i] + 1;
             }else{
                JKYI_LOG_ERROR(g_logger) << "invalid arg idx ="<<i
                                         <<"  val ="<<argv[i];
                return false;
             }
        }else{
           //如果是value的话
           if(now_key){
              add(now_key,argv[i]);
              now_key = nullptr;
           }else{
              JKYI_LOG_ERROR(g_logger) << "invalid arg,idx ="<<i
                                       <<" val ="<<argv[i];
              return false;
           }
        }
    }
    if(now_key){
        add(now_key,"");
    }
    return true;
}

void Env::add(const std::string& key,const std::string& val){
    RWMutexType::WriteLock lock(m_mutex);
    m_args[key] = val;
}
bool Env::has(const std::string& key){
    RWMutexType::ReadLock lock(m_mutex);
    auto it = m_args.find(key);
    return it != m_args.end();
}
void Env::del(const std::string& key){
    RWMutexType::WriteLock lock(m_mutex);
    m_args.erase(key);
}
std::string Env::get(const std::string& key,const std::string& default_val){
    RWMutexType::ReadLock lock(m_mutex);
    auto it = m_args.find(key);
    return it == m_args.end() ? default_val: it->second;
}

void Env::addHelp(const std::string& key,const std::string& val){
    removeHelp(key);
    RWMutexType::WriteLock lock(m_mutex);
    m_helps.push_back(std::make_pair(key,val));
}
void Env::removeHelp(const std::string& key){
    RWMutexType::WriteLock lock(m_mutex);
    for(auto it = m_helps.begin();it != m_helps.end();){
        if(key == it->first){
            it = m_helps.erase(it);
        }else{
            it++;
        }
    }
}
void Env::printHelp(){
    RWMutexType::ReadLock lock(m_mutex);
    std::cout<<"Usage: "<<m_program<<" [options]"<<std::endl;

    for(auto& i : m_helps){
        std::cout << std::setw(5) <<"-"<<i.first<<" : "<<i.second<<std::endl;
    }
    return ;
}

bool Env::setEnv(const std::string& key,const std::string& val){
    return !setenv(key.c_str(),val.c_str(),1);
}
std::string Env::getEnv(const std::string& key,const std::string& default_val){
    const char * rt = getenv(key.c_str());
    if(rt == nullptr){
        return default_val;
    }
    return rt;
}

std::string Env::getAbsolutePath(const std::string& path)const{
    if(path.empty()){
        return "/";
    }

    if(path[0] == '/'){
        return path;
    }
    return m_cwd + path;
}

std::string Env::getAbsoluteWorkPath(const std::string& path)const{
    if(path.empty()){
        return "/";
    }
    if(path[0] == '/'){
        return path;
    }
    static JKYi::ConfigVar<std::string>::ptr g_server_work_path
                    = JKYi::Config::Lookup<std::string>("server.work_path");
    return g_server_work_path->getValue() + "/" + path;
}

std::string Env::getConfigPath(){
    return getAbsolutePath(get("c","conf"));
}

}
