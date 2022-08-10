#include"module.h"
#include"config.h"
#include"env.h"
#include"library.h"
#include"util.h"
#include"log.h"
#include"application.h"

namespace JKYi{

static JKYi::ConfigVar<std::string>::ptr g_module_path = 
                Config::Lookup("module.path",std::string("module"),"module path");

static Logger::ptr g_logger = JKYI_LOG_NAME("system");

Module::Module(const std::string& name,
               const std::string& version,
               const std::string& filename,
               uint32_t type)
    :m_name(name),
     m_version(version),
     m_filename(filename),
     m_id(name + "/" + version),
     m_type(type){
}

void Module::onBeforeArgsParse(int argc,char ** argv){
}
void Module::onAfterArgsParse(int argc,char ** argv){
}

bool Module::handleRequest(JKYi::Message::ptr req,JKYi::Message::ptr rsp,JKYi::Stream::ptr stream){
    JKYI_LOG_DEBUG(g_logger) << "handleRequest req = " << req->toString()
                             << " rsp = " << rsp->toString();
                             
    return true;
}
bool Module::handleNotify(JKYi::Message::ptr notify,JKYi::Stream::ptr stream){
    JKYI_LOG_DEBUG(g_logger) << " handleNotify nty = " << notify->toString();
                             
    return true;
}

bool Module::onLoad(){
    return true;
}
bool Module::onUnload(){
    return true;
}
bool Module::onConnect(JKYi::Stream::ptr stream){
    return true;
}
bool Module::onDisconnect(JKYi::Stream::ptr stream){
    return true;
}

bool Module::onServerReady(){
    return true;
}
bool Module::onServerUp(){
    return true;
}

std::string Module::statusString(){
    std::stringstream ss;
    ss << "Module name = " << getName() 
       << " version = " << getVersion()
       << " filename = " <<getFilename() 
       << std::endl;
    return ss.str();
}

ModuleManager::ModuleManager(){
}

Module::ptr ModuleManager::get(const std::string& name){
    RWMutexType::ReadLock lock(m_mutex);
    auto it = m_modules.find(name);
    return it == m_modules.end() ? nullptr : it->second;
}

void ModuleManager::add(Module::ptr m){
    del(m->getId());
    RWMutexType::WriteLock lock(m_mutex);
    m_modules[m->getId()] = m;
    m_type2Modules[m->getType()][m->getId()] = m;
}
void ModuleManager::del(const std::string& name){
    Module::ptr module;
    RWMutexType::WriteLock lock(m_mutex);
    auto it = m_modules.find(name);
    if(it == m_modules.end()){
        return ;
    }
    module = it->second;
    m_modules.erase(it);
    m_type2Modules[module->getType()].erase(module->getId());
    if(m_type2Modules[module->getType()].empty()){
        m_type2Modules.erase(module->getType());
    }
    lock.unlock();
    module->onUnload();
}

void ModuleManager::delAll(){
    RWMutexType::ReadLock lock(m_mutex);
    auto tmp = m_modules;
    lock.unlock();

    for(auto & i : tmp){
        del(i.first);
    }
}
void ModuleManager::init(){
    std::string path = EnvMgr::GetInstance()->getAbsolutePath(g_module_path->getValue());
    std::vector<std::string>files;
    JKYi::FSUtil::ListAllFile(files,path,".so");

    std::sort(files.begin(),files.end());
    for(auto & i : files){
        initModule(i);
    }
}
void ModuleManager::listByType(uint32_t type,std::vector<Module::ptr>& ms){
    RWMutexType::ReadLock lock(m_mutex);
    auto it = m_type2Modules.find(type);
    if(it == m_type2Modules.end()){
        return ;
    }
    for(auto & i : it->second){
        ms.push_back(i.second);
    }
}

void ModuleManager::foreach(uint32_t type,std::function<void(Module::ptr)>cb){
    std::vector<Module::ptr> ms;
    listByType(type,ms);
    for(auto & m : ms){
        cb(m);
    }
}

void ModuleManager::onConnect(Stream::ptr stream){
    std::vector<Module::ptr> ms;
    listAll(ms);
    for(auto & i : ms){
        i->onConnect(stream);
    }
}

void ModuleManager::onDisconnect(Stream::ptr stream){
    std::vector<Module::ptr> ms;
    listAll(ms);
    for(auto & i : ms){
        i->onDisconnect(stream);
    }
}

void ModuleManager::listAll(std::vector<Module::ptr>& ms){
    RWMutexType::ReadLock lock(m_mutex);
    for(auto & i : m_modules){
        ms.push_back(i.second);
    }
}

void ModuleManager::initModule(const std::string& path){
    Module::ptr module = Library::GetModule(path);
    if(module){
        add(module);
    }
}


}



