#include"my_module.h"
#include"JKYi/config.h"
#include"JKYi/log.h"

namespace name_space{

static JKYi::Logger::ptr g_logger = JKYI_LOG_ROOT();

MyModule::MyModule()
    :JKYi::Module("project_name","1.0",""){
}

bool MyModule::onLoad(){
    JKYI_LOG_INFO(g_logger) << "onload";
    return true;
}

bool MyModule::onUnload(){
    JKYI_LOG_INFO(g_logger) << "onUnload";
    return true;
}

bool MyModule::onServerReady(){
    JKYI_LOG_INFO(g_logger) << "onServerReady";
    return true;
}

bool MyModule::onServerUp(){
    JKYI_LOG_INFO(g_logger) << "onServerUp";
    return true;
}

}
extern "C"{

JKYi::Module* CreateModule(){
   JKYi::Module * module = new name_space::MyModule;
   JKYI_LOG_INFO(name_space::g_logger) << "CreateModule" << module;
   return module;
}

void DestroyModule(JKYi::Module * module){
    JKYI_LOG_INFO(name_space::g_logger) << "DestroyModule";
    delete module;
}

  
}


