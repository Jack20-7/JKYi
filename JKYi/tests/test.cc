#include<iostream>
#include<thread>
#include"../JKYi/log.h"
#include"../JKYi/util.h"
#include"../JKYi/singleton.h"

int main(int argc,char**argv){
    JKYi::Logger::ptr logger(new JKYi::Logger);
    
    logger->addAppender(JKYi::LogAppender::ptr (new JKYi::StdoutLogAppender));
    //然后再写入到日志里面去
    JKYi::LogAppender::ptr fileAppend(new JKYi::FileLogAppender("../log.txt"));
    logger->addAppender(fileAppend);
    //我猜测_FILE_表示的是当前文件。_LINE_表示的是当前行
    //JKYi::LogEvent::ptr event(new JKYi::LogEvent(__FILE__,__LINE__,0,JKYi::GetThreadId(),JKYi::GetFiberId(),time(0)));
    //event->getSS()<<"hello,JKYi log"<<std::endl;

    //logger->log(JKYi::LogLevel::DEBUG,event);
    //std::cout<<"Hello,JKYi  log"<<std::endl;
    JKYI_LOG_DEBUG(logger)<<"Hello,JKYI log!";
    JKYI_LOG_INFO(logger)<<"Hello,JKYI log!";
    JKYI_LOG_ERROR(logger)<<"Hello,JKYI log!";
    JKYI_LOG_WARN(logger)<<"Hello,JKYI log!";
    JKYI_LOG_FATAL(logger)<<"Hello,JKYI log!";
    //
    auto l=JKYi::LoggerMgr::GetInstance()->getLogger("x");
    JKYI_LOG_DEBUG(l)<<"I'm trying to Singleton";
    return 0;
}