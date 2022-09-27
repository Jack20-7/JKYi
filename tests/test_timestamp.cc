#include"JKYi/timestamp.h"
#include"JKYi/log.h"
#include"JKYi/reactor/EventLoop.h"

JKYi::Logger::ptr g_logger = JKYI_LOG_ROOT();

int count = 1;
void run(JKYi::net::EventLoop* loop){
    JKYI_LOG_INFO(g_logger) << count++ ;
    if(count <= 5){
        loop->runAfter(1,
                std::bind(&run,loop));
    }else{
        loop->quit();
    }
}
void run1(){
    JKYI_LOG_INFO(g_logger) << ++count;
}
int main(int argc,char ** argv){
    //JKYi::net::Timestamp timestamp = JKYi::net::Timestamp::now();;
    //JKYI_LOG_INFO(g_logger) << timestamp.toFormattedString();
    JKYi::net::EventLoop loop;
    loop.runAfter(1,
            std::bind(&run,&loop));
    //loop.runEvery(1,&run1);
    loop.loop();
    return 0;
}
