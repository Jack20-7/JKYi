#include"library.h"
#include"JKYi/config.h"
#include"JKYi/env.h"
#include"JKYi/log.h"

#include<dlfcn.h>

namespace JKYi{

static Logger::ptr g_logger = JKYI_LOG_NAME("system");

typedef Module* (*create_module)();
typedef void (*destroy_module)(Module*);

class ModuleCloser{
public:
    ModuleCloser(void * handle,destroy_module d)
        :m_handle(handle),
         m_destroy(d){
    }

    void operator()(Module * module){
        std::string name = module->getName();
        std::string version = module->getVersion();
        std::string path = module->getFilename();
        m_destroy(module);
        int rt = dlclose(m_handle);
        if(rt){
            JKYI_LOG_ERROR(g_logger) << "dlclose handle fail,handle = " <<m_handle
                                     << " name = " << name
                                     << " version = " << version
                                     << " path = " << path
                                     << " error = " <<dlerror();
        }else{
            JKYI_LOG_INFO(g_logger) << "destroy module:" <<name
                                    << " version = " << version
                                    << " path = " << path 
                                    << " handle = " << m_handle
                                    << " success";
        }
    }
private:
    void * m_handle;
    destroy_module m_destroy;
};

//传入.so的路径，对该动态链接库对应的模块进行加载
Module::ptr Library::GetModule(const std::string& path){
    void * handle = dlopen(path.c_str(),RTLD_NOW);
    if(!handle){
        JKYI_LOG_ERROR(g_logger) << "cannot load library path = " <<path
                                 << " error = " << dlerror();
        return nullptr;
    }
    //从该模块中找到名称为CreateModule的函数
    create_module create = (create_module)dlsym(handle,"CreateModule");
    if(!create){
        JKYI_LOG_ERROR(g_logger) << "cannot load symbol CreateModule in " << path
                                 << " error = " << dlerror();
        dlclose(handle);
        return nullptr;
    }
    destroy_module destroy = (destroy_module)dlsym(handle,"DestroyModule");
    if(!destroy){
        JKYI_LOG_ERROR(g_logger) << "cannot load symbol DestroyModule in " << path
                                 << " error = " << dlerror();
        return nullptr;
    }
    Module::ptr module(create(),ModuleCloser(handle,destroy));
    module->setFilename(path);
    JKYI_LOG_INFO(g_logger) << "load module name = " << module->getName()
                            << " version = " << module->getVersion()
                            << " path = " << module->getFilename()
                            << " success";
    Config::LoadFromConfDir(JKYi::EnvMgr::GetInstance()->getConfigPath(),true);
    return module;
}
}
