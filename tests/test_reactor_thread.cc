#include"JKYi/reactor/Thread.h"
#include"JKYi/log.h"
#include"JKYi/reactor/CurrentThread.h"

static JKYi::Logger::ptr g_logger = JKYI_LOG_ROOT();

void run(){
    JKYI_LOG_DEBUG(g_logger) <<" thread fun start";

    JKYI_LOG_DEBUG(g_logger) << " thread id = " << JKYi::CurrentThread::t_cachedTid;
    JKYI_LOG_DEBUG(g_logger) << " thread name = " << JKYi::CurrentThread::t_threadName;

    while(1){}

    JKYI_LOG_DEBUG(g_logger) << "thread fun end";
}
int main(int argc,char ** argv){
    JKYi::net::Thread thread(run);
    thread.start();
    thread.join();
    return 0;
}
