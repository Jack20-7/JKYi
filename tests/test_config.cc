#include"JKYi/log.h"
#include"JKYi/config.h"
#include"JKYi/env.h"
#include<iostream>
#include<yaml-cpp/yaml.h>

JKYi::ConfigVar<int>::ptr g_int_value_config=JKYi::Config::Lookup("system.port",(int)8080,"system port");
JKYi::ConfigVar<float>::ptr g_float_value_config=JKYi::Config::Lookup("system.value",(float)10.2f,"system value");
JKYi::ConfigVar<std::vector<int>>::ptr g_vec_int_config=JKYi::Config::Lookup("system.vec_int",std::vector<int>{1,2,3},"system vec_int");
JKYi::ConfigVar<std::list<int>>::ptr g_list_int_config=JKYi::Config::Lookup("system.list_int",std::list<int>{10,20,30},"system list_int");
JKYi::ConfigVar<std::set<int>>::ptr g_set_int_config=JKYi::Config::Lookup("system.set_int",std::set<int>{5,6,7},"system set_int");
JKYi::ConfigVar<std::unordered_set<int>>::ptr g_uset_int_config=JKYi::Config::Lookup("system.uset_int",std::unordered_set<int>{7,8,9,9},"system uset_int");
JKYi::ConfigVar<std::map<std::string,int>>::ptr g_map_str_int_config=JKYi::Config::Lookup("system.map_str_int",std::map<std::string,int>{{"k",6}},"system map_str_int");
JKYi::ConfigVar<std::unordered_map<std::string,int>>::ptr g_umap_str_int_config=JKYi::Config::Lookup("system.umap_str_int",std::unordered_map<std::string,int>{{"k",9},{"k2",8}},"syetem umap_str_int");
void print_yaml(const YAML::Node&node,int level){
    if(node.IsScalar()){
        //如果当前的这个node是简单类型的话，就可以直接的进行输出
        JKYI_LOG_INFO(JKYI_LOG_ROOT())<<std::string(level*4,' ')<<node.Scalar()<<" - "<<node.Type()<<" - "<<level;
    }else if(node.IsNull()){
        //为空的话
        JKYI_LOG_INFO(JKYI_LOG_ROOT())<<std::string(level*4,' ')<<"NULL - "<<node.Type()<<" - "<<level;
    }else if(node.IsMap()){
        //当前的这个node是map类型
        for(auto it=node.begin();it!=node.end();++it){
            //map类型的话，key是string类型，value依然还是node类型，所以通过递归的方式来进行打印
            JKYI_LOG_INFO(JKYI_LOG_ROOT())<<std::string(level*4,' ')<<it->first<<" - "<<it->second.Type()<<" - "<<level;
            print_yaml(it->second,level+1);
        }
    }else if(node.IsSequence()){
         for(size_t i=0;i<node.size();++i){
             JKYI_LOG_INFO(JKYI_LOG_ROOT())<<std::string(level*4,' ')<<i<<" - "<<node[i].Type()<<" - "<<level;
             print_yaml(node[i],level+1);
         }
    }
}
void test_yaml(){
    //这一步的意思就是使用通过yaml中的loadfile这个函数对配置文件进行加载，并且将加载后的结果放在生成的node类型的root里面
    YAML::Node root=YAML::LoadFile("/home/admin/workSpace/bin/conf/test.yam");
    //JKYI_LOG_INFO(JKYI_LOG_ROOT())<<root;
    print_yaml(root,0);

}
void test_config(){
     JKYI_LOG_INFO(JKYI_LOG_ROOT())<<"before:"<<g_int_value_config->toString();
     JKYI_LOG_INFO(JKYI_LOG_ROOT())<<"before:"<<g_float_value_config->toString();
#define XX(var,name,prefix) \
  {\
     auto&v=var->getValue();\
     for(auto& i:v){     \
         JKYI_LOG_INFO(JKYI_LOG_ROOT())<<#prefix" "<<#name" :"<<i;\
     } \
        JKYI_LOG_INFO(JKYI_LOG_ROOT())<<#prefix" "<<#name" yaml:"<<var->toString();\
  }
  //这个是非序列式的容器的输出
#define XX_M(var,name,prefix)\
  {\
     auto&v=var->getValue();\
     for(auto it=v.begin();it!=v.end();++it){\
        JKYI_LOG_INFO(JKYI_LOG_ROOT())<<#prefix" "#name" :{"\
        <<it->first<<"-"<<it->second<<"}";\
     } \
     JKYI_LOG_INFO(JKYI_LOG_ROOT())<<#prefix" "<<#name" yaml:"<<var->toString();\
  }
     XX(g_vec_int_config,vec_int,before);
     XX(g_list_int_config,list_int,before);
     XX(g_set_int_config,set_int,before);
     XX(g_uset_int_config,uset_int,before);
     XX_M(g_map_str_int_config,map_str_int,before);
     XX_M(g_umap_str_int_config,umap_str_int,before);

     const YAML::Node& root=YAML::LoadFile("/home/admin/workSpace/bin/conf/test.yam");
     JKYi::Config::LoadFromYaml(root);

     JKYI_LOG_INFO(JKYI_LOG_ROOT())<<"after:"<<g_int_value_config->toString();
     JKYI_LOG_INFO(JKYI_LOG_ROOT())<<"after:"<<g_float_value_config->toString();
     XX(g_vec_int_config,vec_int,after);
     XX(g_list_int_config,list_int,after);
     XX(g_set_int_config,set_int,after);
     XX(g_uset_int_config,uset_int,after);
     XX_M(g_map_str_int_config,map_str_int,after);
     XX_M(g_umap_str_int_config,umap_str_int,after);
     
}
class Person{
public:
    std::string m_name;
    int m_age;
    int m_sex;
public:
    Person(std::string name="巫成洋",int age=18,int sex=1)
    :m_name(name),m_age(age),m_sex(sex){}
    std::string toString()const {
          std::stringstream ss;
          ss<<"[Person name="<<m_name
          <<" age="<<m_age
          <<" sex="<<m_sex
          <<" ]";
          //
          return ss.str();
     }
     bool operator== (const Person&rhv)const {
         return this->m_name==rhv.m_name
                &&this->m_age==rhv.m_age
                &&this->m_sex==rhv.m_sex;
    }
};
namespace JKYi{
//举例说明的Person自定义数据类型的重载
//string->Person之间的转化
template<>
class LexicalCast<std::string,Person>{
public:
    Person operator()(const std::string&s){
       const YAML::Node&node=YAML::Load(s);
       Person p;
       p.m_name=node["name"].as<std::string>();
       p.m_age=node["age"].as<int>();
       p.m_sex=node["sex"].as<int>();
       //
       return p;
    }
};
//Person ->string
template<>
class LexicalCast<Person,std::string>{
public:
   std::string operator()(Person&p) {
      YAML::Node node;
      node["name"]=p.m_name;
      node["age"]=p.m_age;
      node["sex"]=p.m_sex;
      std::stringstream ss;
      ss<<node;
      return ss.str();
   }
};
}
JKYi::ConfigVar<Person>::ptr g_person_config=JKYi::Config::Lookup("class.person",Person(),"class person");
void test_class(){
    //测试用户自定义数据类型
    //默认的配置系统仅基本类型、vector、list、set、unordered_set、map、unordered_map
    //用户如果想要支持自定义数据类型的话，需要用户自己在Config.cc里面对LexicalCast进行特化
    //就拿person类型举例
    JKYI_LOG_INFO(JKYI_LOG_ROOT())<<"before: "<<g_person_config->getValue().toString()<<" - "<<g_person_config->toString();
    //测试事件修改机制
    g_person_config->addListener([](const Person&lhv,const Person&rhv){
        JKYI_LOG_INFO(JKYI_LOG_ROOT())<<"old value:"<<lhv.toString()<<" new value:"<<rhv.toString();
    });
    const YAML::Node& root=YAML::LoadFile("/home/admin/workSpace/bin/conf/test.yam");
    JKYi::Config::LoadFromYaml(root);
    JKYI_LOG_INFO(JKYI_LOG_ROOT())<<"after: "<<g_person_config->getValue().toString()<<" - "<<g_person_config->toString();
}
void test_log(){
    JKYi::Logger::ptr system_log=JKYI_LOG_NAME("system");
    JKYI_LOG_INFO(system_log)<<"hello,JKYI log";
    std::cout<<JKYi::LoggerMgr::GetInstance()->toYamlString()<<std::endl;
    const YAML::Node&node=YAML::LoadFile("/home/admin/workSpace/bin/conf/log.yml");
    JKYi::Config::LoadFromYaml(node);
    std::cout<<"----------------------------------------------"<<std::endl;
    std::cout<<JKYi::LoggerMgr::GetInstance()->toYamlString()<<std::endl;
    JKYI_LOG_INFO(system_log)<<"hello,JKYI log";
    JKYI_LOG_INFO(system_log)<<"hello,JKYI log";
    JKYI_LOG_INFO(system_log)<<"hello,JKYI log";
    JKYI_LOG_INFO(system_log)<<"hello,JKYI log";
}

void test_loadconf(){
    JKYi::Config::LoadFromConfDir("conf");
}
int main(int argc,char **argv){
   
    //test_yaml();
    //test_config();
    //test_class();
    //test_log();
    JKYi::EnvMgr::GetInstance()->init(argc,argv);
    test_loadconf();
    sleep(10);
    test_loadconf();

	JKYi::Config::Visit([](JKYi::ConfigVarBase::ptr p){
         JKYI_LOG_INFO(JKYI_LOG_ROOT())<<"name="<<p->getName()
		                              <<" description="<<p->getDescription()
									  <<" typename="<<p->getType()
									  <<" value= "<<p->toString();
	});
    return 0;
}
