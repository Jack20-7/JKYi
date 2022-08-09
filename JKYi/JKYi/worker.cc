#include"worker.h"
#include"log.h"
#include"config.h"
#include"util.h"

#include<string.h>

namespace JKYi{

static Logger::ptr g_logger = JKYI_LOG_NAME("system");

static ConfigVar<std::unordered_map<std::string,std::unordered_map<std::string,std::string>> >::ptr 
                 g_workers_config = Config::Lookup("workers",
                    std::unordered_map<std::string,std::unordered_map<std::string,std::string> >(),
                    "worker config");

WorkerManager::WorkerManager()
    :m_stop(false){
}

void WorkerManager::add(Scheduler::ptr s){
    m_datas[s->getName()].push_back(s);
}

Scheduler::ptr WorkerManager::get(const std::string& name){
    auto it = m_datas.find(name);
    if(it == m_datas.end()){
        return nullptr;
    }

    if(it->second.size() == 1){
        return it->second[0];
    }
    return it->second[rand() % it->second.size()];
}

IOManager::ptr WorkerManager::getAsIOManager(const std::string& name){
    return std::dynamic_pointer_cast<IOManager> (get(name));
}

bool WorkerManager::init(const std::unordered_map<std::string,
                                        std::unordered_map<std::string,std::string>>& v){
   for(auto& i : v){
       std::string name = i.first;
       int32_t thread_num = GetParamValue(i.second,"thread_num",1);
       int32_t worker_num = GetParamValue(i.second,"worker_num",1);
       //JKYI_LOG_DEBUG(g_logger) << "worker name = " << name << " is created";

       for(int32_t x = 0;x < worker_num;++x){
           Scheduler::ptr s;
           if(!x){
               s = std::make_shared<IOManager>(thread_num,false,name);
           }else{
               s = std::make_shared<IOManager>(thread_num,false,name + "-" + std::to_string(x));
           }
           add(s);
       }
   }
   m_stop = m_datas.empty();
   return true;
}

bool WorkerManager::init(){
    auto workers = g_workers_config->getValue();
    return init(workers);
}

void WorkerManager::stop(){
    if(m_stop){
        return ;
    }
    for(auto& i : m_datas){
        for(auto& n : i.second){
            n->schedule([](){});
            n->stop();
        }
    }
    m_datas.clear();
    m_stop = true;
}

uint32_t WorkerManager::getCount(){
    return m_datas.size();
}

std::ostream& WorkerManager::dump(std::ostream& os){
    for(auto& i : m_datas){
        for(auto& x : i.second){
            x->dump(os) << std::endl;
        }
    }
    return os;
}
                                               
}
