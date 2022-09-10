#include"JKYi/db/redis.h"
#include"JKYi/log.h"
#include"JKYi/config.h"
#include"JKYi/iomanager.h"

#include<string.h>

JKYi::Logger::ptr g_logger = JKYI_LOG_ROOT();
void run(){
    JKYi::IRedis::ptr rds = JKYi::RedisMgr::GetInstance()->get("redis");
    if(!rds){
        JKYI_LOG_ERROR(g_logger) << " get redis error " ;
        return ;
    }
    JKYI_LOG_INFO(g_logger) << " get foxredis ok";
    for(int i = 0;i < 10;++i){
        rds->cmd("set %d %d",i,i);
        JKYI_LOG_INFO(g_logger) << " cmd ---- ";
    }
    for(int i = 0;i < 10;++i){
        auto reply = rds->cmd("get %d",i);
        if(reply){
            if(reply->type == REDIS_REPLY_INTEGER){
                JKYI_LOG_INFO(g_logger) << reply->integer;
            }else if(reply->type == REDIS_REPLY_STRING){
                JKYI_LOG_INFO(g_logger) << reply->str;
            }
        }
    }
    return ;
}
int main(int argc,char ** argv){
    JKYi::Config::LoadFromConfDir("conf");
    JKYi::FoxThreadMgr::GetInstance()->init();
    JKYi::FoxThreadMgr::GetInstance()->start();

    JKYi::IOManager iom(1);
    iom.schedule(&run);
    return 0;
}
