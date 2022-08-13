#ifndef _JKAI_LOG_H_
#define _JKAI_LOG_H_

#include <string>
#include <stdint.h>
#include <memory>
#include <list>
#include <sstream>
#include <fstream>
#include <vector>
#include <stdarg.h>
#include <map>
#include<yaml-cpp/yaml.h>

#include"mutex.h"
#include"thread.h"
#include "singleton.h"
#include "util.h"

//定义一些宏，方便我们对日志的使用
#define JKYI_LOG_LEVEL(logger,level) \
    if(logger->getLevel()<=level) \
       JKYi::LogEventWrap(JKYi::LogEvent::ptr (new JKYi::LogEvent(logger,level,\
       __FILE__,__LINE__,0,JKYi::GetThreadId(),\
       JKYi::GetFiberId(),time(0),JKYi::Thread::GetName()))).getSS()

//
#define JKYI_LOG_DEBUG(logger) JKYI_LOG_LEVEL(logger,JKYi::LogLevel::DEBUG)
//
#define JKYI_LOG_INFO(logger) JKYI_LOG_LEVEL(logger,JKYi::LogLevel::INFO)
//
#define JKYI_LOG_WARN(logger) JKYI_LOG_LEVEL(logger,JKYi::LogLevel::WARN)
//
#define JKYI_LOG_ERROR(logger) JKYI_LOG_LEVEL(logger,JKYi::LogLevel::ERROR)
//
#define JKYI_LOG_FATAL(logger) JKYI_LOG_LEVEL(logger,JKYi::LogLevel::FATAL)
//这里为了能够方便的对logger进行拿取，再次定义一个宏
#define JKYI_LOG_ROOT() JKYi::LoggerMgr::GetInstance()->getRoot()
//定义一个根据name在loggermanager中找到对应的logger的宏
#define JKYI_LOG_NAME(name) JKYi::LoggerMgr::GetInstance()->getLogger(name)

namespace JKYi{

class Logger;
class LoggerManager;

//日志级别
class LogLevel{
public:
   enum Level{
     UNKNOW=0,
     DEBUG=1,
     INFO=2,
     WARN=3,
     ERROR=4,
     FATAL=5
   };
   static const char* ToString(LogLevel::Level level);
   static LogLevel::Level FromString(const std::string&str);
};
//日志事件
class LogEvent{
public:
   typedef std::shared_ptr<LogEvent> ptr; 
   LogEvent(std::shared_ptr<Logger>logger
           ,LogLevel::Level level
           ,const char*file
		   ,int32_t line
		   ,uint32_t elapse
		   ,uint32_t thread_id
		   ,uint32_t fiber_id
		   ,uint32_t time
		   ,const std::string&threadName);
   //提供给外界得接口
   const char* getFile()const {return m_file;}
   int32_t getLine()const {return m_line;}
   uint32_t getElapse()const {return m_elapse;}
   uint32_t getThreadId()const {return m_threadId;}
   uint32_t getFiberId()const {return m_fiberId;}
   uint32_t getTime()const{return m_time;}
   std::string getContent() const{return m_ss.str();}
   std::stringstream& getSS() { return m_ss;}
   std::shared_ptr<Logger> getLogger()const {return m_logger;}
   LogLevel::Level getLevel() const {return m_level;}
   const std::string& getThreadName()const {return m_threadName;}
private:
   const char* m_file=nullptr;//目标日志文件的名称
   int32_t m_line=0;//行号
   uint32_t m_elapse=0;//程序启动开始到现在的毫秒数
   uint32_t m_threadId=0;//线程ID
   uint32_t m_fiberId=0;//协程ID
   uint32_t m_time=0;//时间戳
   std::stringstream m_ss;//流，用来存储要输出的日志
   //下面的内容是用来辅助日志的输出
   std::shared_ptr<Logger>m_logger;
   LogLevel::Level m_level;
   //记录当前线程的名称
   std::string m_threadName;
};
//定义一个日志包装器，用于宏定义的使用
class LogEventWrap{
public:
   LogEventWrap(LogEvent::ptr event);
   ~LogEventWrap();
   LogEvent::ptr getEvent()const {return m_event;}
   std::stringstream& getSS();
private:
   LogEvent::ptr m_event;
};
//日志格式
class LogFormatter{
public:
    typedef std::shared_ptr<LogFormatter> ptr;

    LogFormatter(const std::string&pattern);
    std::string format(std::shared_ptr<Logger>logger,LogLevel::Level level,LogEvent::ptr event);
public:
    //分别用来对不同的类型进行输出，比如%d %l等
    class FormatItem{
    public:
       typedef std::shared_ptr<FormatItem> ptr;
       //FormatItem(const std::string&fmt=""){}
       virtual ~FormatItem(){}
       virtual void format(std::ostream&os,std::shared_ptr<Logger>logger,LogLevel::Level level,LogEvent::ptr event)=0;
    };
    //pattern的解析
    void init();
    //
    bool isError()const{return m_error;}

    const std::string& getPattern()const{
        return m_pattern;
    }
private:
    //标志当前日志输出的格式
    std::string m_pattern;

    std::vector<FormatItem::ptr>m_items;

    //判断格式是否正确
    bool m_error=false;
};
//
//日志输出地
class LogAppender{
friend class Logger;
public:
    typedef std::shared_ptr<LogAppender> ptr;
	typedef SpinLock MutexType;
    virtual ~LogAppender(){};//由于该类需要作为基类，所以这里的话就析构函数申明为虚函数，避免内存泄漏
    virtual void log(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) = 0;
    //
    virtual std::string toYamlString()=0;
    //设置日志输出的格式
    void setFormatter(LogFormatter::ptr formatter);
    LogFormatter::ptr getFormatter(); 
    //级别
    LogLevel::Level getLevel(LogLevel::Level level){return m_level;}
    void setLevel(LogLevel::Level level){m_level=level;}
    //
protected://这里由于子类可能会用到，所以权限设置为protected
    
    LogLevel::Level m_level=LogLevel::DEBUG;//默认为debug模式
    
    LogFormatter::ptr m_formatter;//定义日志输出的格式
    //
    bool m_hasFormatter=false;
	//互斥锁
	MutexType m_mutex;
};
//日志器
//只有继承该类之后才能够在成员函数中使用shared_from_this
//提供给用户的接口，用户相当于是通过该类来对日志进行输出的
class Logger:public std::enable_shared_from_this<Logger>{
friend class LoggerManager;
public:
    typedef std::shared_ptr<Logger> ptr;
	typedef SpinLock MutexType;
    Logger(const std::string& name="root");
    void log(LogLevel::Level level,LogEvent::ptr event);

    //用户输出日志时调用的接口
    void debug(LogEvent::ptr event);//输出debug级别的日志
    void info(LogEvent::ptr event);
    void warn(LogEvent::ptr event);
    void error(LogEvent::ptr event);
    void fatal(LogEvent::ptr event);

    //添加删除日志的输出地
    void addAppender(LogAppender::ptr appender);
    void delAppender(LogAppender::ptr appender);
    void clearAppender();
    //
    void setFormatter(LogFormatter::ptr formatter);
    void setFormatter(const std::string &str);
    LogFormatter::ptr getFormatter();
    //
    LogLevel::Level getLevel()const {return m_level;}
    void setLevel(LogLevel::Level level){m_level=level;}

    std::string getName()const {return m_name;}

    //用于调试的时候使用，将logger中的信息以yaml格式的配置形式返回
    std::string toYamlString();
private:
    std::string m_name;       //日志的名称

    LogLevel::Level m_level;  //日志的级别，只有满足级别的日志才会被输出workSpace/JKYi/log.h

    std::list<LogAppender::ptr>m_appenders;//Appender集合

    LogFormatter::ptr m_formatter;//日志的输出格式

    Logger::ptr m_root;
	//
	MutexType m_mutex;
};


//将日志输出到控制台
class StdoutLogAppender:public LogAppender{
friend class Logger;//友元关系不能够继承
public:
    typedef std::shared_ptr<StdoutLogAppender> ptr;
    void log(Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override;
    std::string toYamlString()override;
};
//将日志输出到文件
class FileLogAppender:public LogAppender{
friend class Logger;
public:
     typedef std::shared_ptr<FileLogAppender> ptr;
   
     FileLogAppender(const std::string& filename);
     void log(Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override;
     std::string toYamlString()override;
     bool reopen();//重新对文件进行打开，打开成功返回true
private:
     std::string m_filename;//文件名
     std::ofstream m_filestream;
	 uint64_t m_lastTime=0;//记录上一次打开的时间
};
//为了方便使用日志而定义得类，使得我们不需要每一次在打日志得时候都需要去自己手动定义一个logger
class LoggerManager{
public:
     typedef SpinLock MutexType;
     LoggerManager();
     Logger::ptr getLogger(const std::string&name);
     void init();
     Logger::ptr getRoot()const {return m_root;}
     std::string toYamlString();
private:
    std::map<std::string,Logger::ptr>m_loggers;
    //默认的主日志器
    Logger::ptr m_root;
	//
	MutexType m_mutex;
};

//通过单例模式来对LoggerManager进行管理
typedef JKYi::Singleton<LoggerManager> LoggerMgr;

}

#endif
