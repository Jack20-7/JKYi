#ifndef _JKYI_WORKER_H_
#define _JKYI_WORKER_H_

#include<unordered_map>
#include"mutex.h"
#include"singleton.h"
#include"log.h"
#include"iomanager.h"

namespace JKYi{

class WorkerManager{
public:
    typedef std::shared_ptr<WorkerManager> ptr;

    WorkerManager();
    void add(Scheduler::ptr s);
    Scheduler::ptr get(const std::string& name);
    IOManager::ptr getAsIOManager(const std::string& name); 

    template<class FiberOrCb>
    void schedule(const std::string& name,FiberOrCb fc,int thread = -1){
        auto s = get(name);
        if(s){
            s->schedule(fc,thread);
        }else{
            static Logger::ptr g_logger = JKYI_LOG_NAME("system");
            JKYI_LOG_ERROR(g_logger) << "schedule name= " << name
                                     << " not exists";
        }
        return ;
    }
    template<class Iter>
    void schedule(const std::string& name,Iter begin,Iter end){
        auto s = get(name);
        if(s){
            s->schedule(begin,end);
        }else{
            static Logger::ptr g_logger = JKYI_LOG_NAME("system");
            JKYI_LOG_ERROR(g_logger) << "schedule name=" << name
                                     << " not exists";
        }
        return ;
    }

    bool init();
    bool init(const std::unordered_map<std::string,std::unordered_map<std::string,std::string> >& v);
    void stop();

    bool isStop()const { return m_stop; }
    std::ostream& dump(std::ostream& os);

    uint32_t getCount();
private:
    //名称 - 协程调度器数组
    std::map<std::string,std::vector<Scheduler::ptr> >m_datas;
    bool m_stop;
};

typedef JKYi::Singleton<WorkerManager> WorkerMgr;
}
#endif
