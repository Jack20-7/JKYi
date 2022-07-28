#include"JKYi/JKYi.h"

static JKYi::Logger::ptr g_logger = JKYI_LOG_ROOT();

JKYi::Timer::ptr g_timer;
int server_main(int argc,char ** argv){
    JKYI_LOG_INFO(g_logger)<<JKYi::ProcessInfoMgr::GetInstance()->toString();
    JKYi::IOManager iom(1);
    g_timer = iom.addTimer(1000,[](){
        JKYI_LOG_INFO(g_logger)<<"onTimer";
        static int count = 0;
        if(++count > 20){
          g_timer->cancel();
        }
    },true);
    return 0;
}
int main(int argc,char ** argv){
    return JKYi::start_daemon(argc,argv,server_main,argc != 1);
}
