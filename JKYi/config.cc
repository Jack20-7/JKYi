#include"config.h"
#include"util.h"
#include"env.h"
#include"log.h"

#include<sys/types.h>
#include<sys/stat.h>
#include<unistd.h>
namespace JKYi{
//查找config的map中是否存在name这个名称的
ConfigVarBase::ptr Config::LookupBase(const std::string&name){
  RWMutexType::ReadLock lock(getMutex());
  auto it=getDatas().find(name);
  return it==getDatas().end()?nullptr:it->second;
}

static JKYi::Logger::ptr g_logger = JKYI_LOG_NAME("system");
//该函数的作用就是将
// A：
//  B：10
// 转化为
//A.B 10
static void ListAllMember(const std::string&prefix,const YAML::Node&node,std::list<std::pair<std::string,const YAML::Node>>&output){
    if(prefix.find_first_not_of("abcdefghikjlmnopqrstuvwxyz._012345678")!=std::string::npos){
        //如果名字有问题
        JKYI_LOG_ERROR(JKYI_LOG_ROOT())<<"Config invalid name: "<<prefix<<": "<<node;
        return ;
    }
    output.push_back({prefix,node});
    //
    if(node.IsMap()){
        for(auto it = node.begin();it != node.end();++it){
            ListAllMember(prefix.empty() ? it->first.Scalar(): prefix+"." + it->first.Scalar(),it->second,output);
        }
    }
}

void Config::LoadFromYaml(const YAML::Node& node){
    std::list<std::pair<std::string,const YAML::Node>>all_nodes;
    ListAllMember("",node,all_nodes);
    for(auto&i : all_nodes){
        std::string key=i.first;
        if(key.empty()){
            continue;
        }
        //转小写，其实这一步没有啥必要，因为all_nodes里面的pair中的string都是通过上面那个函数生成的，而上面那个函数会进行验证的
        std::transform(key.begin(),key.end(),key.begin(),::tolower);
        ConfigVarBase::ptr var = LookupBase(key);
        if(var){
            if(i.second.IsScalar()){
                //如果是简单类型的话，就直接将值写入到对应的ConfigVar里面的m_val中去
                var->fromString(i.second.Scalar());
            }else{
                //而如果不是简单类型的话，就需要通过流的方式进行转型写入
                std::stringstream ss;
                ss<<i.second;
                var->fromString(ss.str());
            }
        }
    }
}
  
//为了实现当配置文件发生变化时才重新加载
//文件名 - 最近一次修改的s时间
static std::map<std::string,uint64_t> s_file2modifytime;
static JKYi::Mutex s_mutex;   

void Config::LoadFromConfDir(const std::string& path,bool force){
    std::string absolute_path = JKYi::EnvMgr::GetInstance()->getAbsolutePath(path); 
    JKYI_LOG_DEBUG(g_logger) << absolute_path;

    std::vector<std::string>files;
    FSUtil::ListAllFile(files,absolute_path,".yml");

    for(auto& i : files){
      {
          struct stat st;
          lstat(i.c_str(),&st);
          JKYi::Mutex::Lock lock(s_mutex);
          //如果该文件没有修改过的话
          if(!force && s_file2modifytime[i] == (uint64_t)st.st_mtime){
              continue;
          }
          s_file2modifytime[i] = st.st_mtime;
      }
      try{
          YAML::Node root = YAML::LoadFile(i);
          LoadFromYaml(root);
          JKYI_LOG_INFO(g_logger)<<" LoadFromFile file ="<< i <<" OK";
      }catch(...){
          JKYI_LOG_ERROR(g_logger)<<"LoadFromFile file ="<< i <<" failed";
      }

    }

}
void Config::Visit(std::function<void (ConfigVarBase::ptr)>cb){
   RWMutexType::ReadLock lock(getMutex());
   ConfigVarMap&m=getDatas();
   for(auto it=m.begin();it!=m.end();++it){
	   cb(it->second);
   }

}


}
