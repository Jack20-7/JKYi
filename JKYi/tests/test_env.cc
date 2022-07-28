#include"JKYi/env.h"
#include"JKYi/log.h"

#include<unistd.h>
#include<iostream>
#include<fstream>


static JKYi::Logger::ptr g_logger = JKYI_LOG_ROOT();
int main(int argc,char ** argv){
    JKYi::EnvMgr::GetInstance()->addHelp("s","start with termal");
    JKYi::EnvMgr::GetInstance()->addHelp("d","run as daemon");
    JKYi::EnvMgr::GetInstance()->addHelp("p","print help");
    if(!JKYi::EnvMgr::GetInstance()->init(argc,argv)){
        JKYi::EnvMgr::GetInstance()->printHelp();
        return 0;
    }
    if(JKYi::EnvMgr::GetInstance()->has("p")){
        JKYi::EnvMgr::GetInstance()->printHelp();
    }

    //测试环境变量的设置
    JKYI_LOG_INFO(g_logger) <<"exe= "<<JKYi::EnvMgr::GetInstance()->getExe();
    JKYI_LOG_INFO(g_logger) <<"cwd= "<<JKYi::EnvMgr::GetInstance()->getCwd();

    JKYI_LOG_INFO(g_logger) <<"path= "<<JKYi::EnvMgr::GetInstance()->getEnv("PATH");
    JKYI_LOG_INFO(g_logger) <<"test= "<<JKYi::EnvMgr::GetInstance()->getEnv("TEST","wwwwww");
    JKYI_LOG_INFO(g_logger) <<"set env  "<<JKYi::EnvMgr::GetInstance()->setEnv("TEST","YY");
    JKYI_LOG_INFO(g_logger) <<"test= "<<JKYi::EnvMgr::GetInstance()->getEnv("TEST");
    return 0;
}
