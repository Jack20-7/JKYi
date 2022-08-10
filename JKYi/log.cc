#include "log.h"
#include <map>
#include <iostream>
#include <fstream>
#include <functional>
#include <time.h>
#include <string>
#include "config.h"
#include "util.h"

namespace JKYi{
    //宏定义中的##表示将两个字符串连在一起，#用来将传入的参数转换为字符串
    //宏定义中的\的目的就是为了将一行拆分为多行
    //该函数的作用就是传入一个级别，然后就会返回该级别对应的字符串类型
 const char* LogLevel::ToString(LogLevel::Level level){
      switch(level){
        #define XX(name) \
             case LogLevel::name: \
               return #name;\
               break;
        
          XX(DEBUG);
          XX(INFO);
          XX(WARN);
          XX(ERROR);
          XX(FATAL);
        #undef XX
              default:
                return "UNKNOW";
      }
      return "UNKNOW";
 }
 LogLevel::Level LogLevel::FromString(const std::string&str){
    #define XX(name,s)\
      {\
         if(str==#s){\
            return LogLevel::name;\
         }\
      }
      //支持小写的level
       XX(DEBUG,debug);
       XX(INFO,info);
       XX(WARN,warn);
       XX(ERROR,error);
       XX(FATAL,fatal);
       //
       XX(DEBUG,DEBUG);
       XX(INFO,INFO);
       XX(WARN,WARN);
       XX(ERROR,ERROR);
       XX(FATAL,FATAL);
    #undef XX
    return LogLevel::UNKNOW;
 }
 //logger包装器
LogEventWrap::LogEventWrap(LogEvent::ptr event)
 :m_event(event){}
 //因为再通过宏定义输出日志的时候，创建的LogEvent是临时对象，再当前行结束时就会被销毁
 //所以这里就再析构函数中对写入日志流的数据进行析构
LogEventWrap::~LogEventWrap(){
     m_event->getLogger()->log(m_event->getLevel(),m_event);
 }
std::stringstream& LogEventWrap::getSS(){
    return m_event->getSS();
}
//接下来定义一些日志输出的格式，用来辅助日志的输出
//日志消息部分的输出类
class MessageFormatItem:public LogFormatter::FormatItem{
public:
   MessageFormatItem(const std::string& str=""){}
   void format(std::ostream&os,Logger::ptr logger,LogLevel::Level level,LogEvent::ptr event)override{
      os<<event->getContent();
   }
};
//日志级别的输出类
class LevelFormatItem:public LogFormatter::FormatItem{
public:
    LevelFormatItem(const std::string&str=""){}
    void format(std::ostream&os,Logger::ptr logger,LogLevel::Level level,LogEvent::ptr event)override{
       os<<LogLevel::ToString(level);  
    }
};
//输出持续时间的类
class ElapseFormatItem:public LogFormatter::FormatItem{
public:
    ElapseFormatItem(const std::string&str=""){}
    void format(std::ostream&os,Logger::ptr logger,LogLevel::Level level,LogEvent::ptr event)override{
      os<<event->getElapse();
    }
};
//输出名称的类
class NameFormatItem:public LogFormatter::FormatItem{
public:
    NameFormatItem(const std::string&str=""){}
    void format(std::ostream&os,Logger::ptr logger,LogLevel::Level level,LogEvent::ptr event)override{
      os<<event->getLogger()->getName();  //打印出最原始的logger
    }
};
//输出线程ID的类
class ThreadIdFormatItem:public LogFormatter::FormatItem{
public:
    ThreadIdFormatItem(const std::string&str=""){}
    void format(std::ostream&os,Logger::ptr logger,LogLevel::Level level,LogEvent::ptr event)override{
      os<<event->getThreadId();
    }
};
//输出协程ID的类
class FiberIdFormatItem:public LogFormatter::FormatItem{
public:
    FiberIdFormatItem(const std::string&str=""){}
    void format(std::ostream&os,Logger::ptr logger,LogLevel::Level level,LogEvent::ptr event)override{
      os<<event->getFiberId();
    }
};
//输出文件名的类
class FilenameFormatItem : public LogFormatter::FormatItem {
public:
    FilenameFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << event->getFile();
    }
};
//
// class DateTimeFormatItem:public LogFormatter::FormatItem{
// public:
//     DateTimeFormatItem(const std::string& format = "%Y-%m-%d %H:%M:%S");
//       :m_format(format){
//           if(m_format.empty()){ m_format = "%Y-%m-%d %H:%M:%S";}
//       }
//     void format(std::ostream&os,Logger::ptr logger,LogLevel::Level level,LogEvent::ptr event)override{
//         os<<event->getTime();
//     }
// private:
//     std::string m_format;
// };
//日志打印时输出当前的时间
class DateTimeFormatItem : public LogFormatter::FormatItem {
public:
    DateTimeFormatItem(const std::string& format = "%Y-%m-%d %H:%M:%S")
        :m_format(format) {
        if(m_format.empty()) {
            m_format = "%Y-%m-%d %H:%M:%S";
        }
    }

    void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
        //用来返回当前时间
        struct tm tm;
        time_t time = event->getTime();
        localtime_r(&time, &tm);
        char buf[64];
        strftime(buf, sizeof(buf), m_format.c_str(), &tm);
        os << buf;
       // os<<event->getTime();
    }
private:
    std::string m_format;
};
//输出当前所在行的类
class LineFormatItem : public LogFormatter::FormatItem {
public:
    LineFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << event->getLine();
    }
};
//换行的类
class NewLineFormatItem : public LogFormatter::FormatItem {
public:
    NewLineFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << std::endl;
    }
};
//
class StringFormatItem:public LogFormatter::FormatItem{
public:
   StringFormatItem(const std::string&str)
     :m_string(str){}
     void format(std::ostream&os,Logger::ptr logger,LogLevel::Level level, LogEvent::ptr event)override{
         os<<m_string;
     }
private:
   std::string m_string;
};
//
class TabFormatItem:public LogFormatter::FormatItem{
public:
   TabFormatItem(const std::string str=""){}
   void format(std::ostream&os,Logger::ptr logger,LogLevel::Level level,LogEvent::ptr event)override{
       os<<"\t";
   }
private:
   std::string m_string;
};
//打印出线程的名称
class ThreadNameFormatItem: public LogFormatter::FormatItem {
public:
    ThreadNameFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << event->getThreadName();
    }
};

LogEvent::LogEvent(std::shared_ptr<Logger>logger
                   ,JKYi::LogLevel::Level level 
				   ,const char*file
				   ,int32_t line
				   ,uint32_t elapse
				   ,uint32_t thread_id
				   ,uint32_t fiber_id
				   ,uint32_t time
				   ,const std::string& threadName)
  :m_file(file)
  ,m_line(line)
  ,m_elapse(elapse)
  ,m_threadId(thread_id)
  ,m_fiberId(fiber_id)
  ,m_time(time)
  ,m_logger(logger)
  ,m_level(level)
  ,m_threadName(threadName){}
//------------------------------------------------------------Logger-------------------------------------------------------------------
Logger::Logger(const std::string&name)
    :m_name(name)
    ,m_level(LogLevel::DEBUG){       //%d表示时间   %p表示级别  %f表示文件名   %l表示行号  %m表示消息  %n表示换行
        m_formatter.reset(new LogFormatter("%d{%Y-%m-%d %H:%M:%S}%T%t%T%N%T%F%T[%p]%T[%c]%T%f:%l%T%m%n"));
    }

void Logger::addAppender(LogAppender::ptr appender){
	MutexType::Lock lock(m_mutex);
    if(!appender->getFormatter()){
		MutexType::Lock lock(appender->m_mutex);
        //这里直接给这个appender设置formatter，不使用setFormtter。这样相当于一个appedner如果在使用所在的logger的formmater的话，那么他的m_hasFormatter=false
        appender->m_formatter=m_formatter;
    }
    m_appenders.push_back(appender);
}

void Logger::delAppender(LogAppender::ptr appender){
	//
	MutexType::Lock lock(m_mutex);
    for(auto it=m_appenders.begin();it!=m_appenders.end();++it){
        if((*it)==appender){
            m_appenders.erase(it);
            break;
        }
    }
}
void Logger::clearAppender(){
	MutexType::Lock lock(m_mutex);
     m_appenders.clear();
}
//该函数就是用来向集合中的所有的地址输出日志
void Logger::log(LogLevel::Level level,LogEvent::ptr event){
    if(level>=m_level){
        auto self=shared_from_this();//返回一个执行自己的shared_ptr
		MutexType::Lock lock(m_mutex);
        if(!m_appenders.empty()){
            for(auto&i:m_appenders){
            i->log(self,level,event);//实际调用的是Appender的接口来输出的日志
          }
        }else if(m_root){
            m_root->log(level,event);
        }
    }
    return ;
}
//
void Logger::setFormatter(LogFormatter::ptr formatter){
   MutexType::Lock lock(m_mutex);
   m_formatter=formatter;
   //这里的logger还需要对在addAppender时使用它的formatter的下游appender负责
   for(auto&i:m_appenders){
	   //这里还需要加appender的锁
	   MutexType::Lock lock(i->m_mutex);
       if(!i->m_hasFormatter){
           i->m_formatter=formatter;
       }
   }
}
void Logger::setFormatter(const std::string&str){
    JKYi::LogFormatter::ptr new_val(new JKYi::LogFormatter(str));
    if(new_val->isError()){
        //如果格式有问题的话
        std::cout<<"Logger setFormatter name:"<<m_name
        <<" value: "<<str
        <<" inValid formatter"<<std::endl; 
        return ;
    }
    //如果合法的话
    //m_formatter=new_val;
    setFormatter(new_val);
    return ;
}

//
std::string Logger::toYamlString(){
   //转换的一个思想就是先转化为node，在通过流的方式返回string
   MutexType::Lock lock(m_mutex);
   YAML::Node node;
   node["name"]=m_name;
   if(m_level!=LogLevel::UNKNOW){
       node["level"]=LogLevel::ToString(m_level);
   }
   if(m_formatter){
       node["formatter"]=m_formatter->getPattern();
   }
   for(auto &i :m_appenders){
       node["appender"].push_back(YAML::Load(i->toYamlString()));
   }
   std::stringstream ss;
   ss<<node;
   return ss.str();

}
LogFormatter::ptr Logger::getFormatter(){
	MutexType::Lock lock(m_mutex);
    return m_formatter;
}

void Logger::debug(LogEvent::ptr event){
   log(LogLevel::DEBUG,event);
}
void Logger::info(LogEvent::ptr event){
   log(LogLevel::INFO,event);
}
void Logger::warn(LogEvent::ptr event){
   log(LogLevel::WARN,event);
}
void Logger::error(LogEvent::ptr event){
   log(LogLevel::ERROR,event);
}
void Logger::fatal(LogEvent::ptr event){
   log(LogLevel::FATAL,event);
}

//
void LogAppender::setFormatter(LogFormatter::ptr formatter){
	//
	MutexType::Lock lock(m_mutex);
   m_formatter=formatter;
   if(m_formatter){
       m_hasFormatter=true;
   }else{
       m_hasFormatter=false;
   }
   return ;
}
LogFormatter::ptr LogAppender::getFormatter(){
  MutexType::Lock lock(m_mutex);
  return m_formatter;
}
void StdoutLogAppender::log(std::shared_ptr<Logger>logger,LogLevel::Level level,LogEvent::ptr event){
    if(level>=m_level){
		MutexType::Lock lock(m_mutex);
        std::cout<<m_formatter->format(logger,level,event);
    }
}
std::string StdoutLogAppender::toYamlString(){
	MutexType::Lock lock(m_mutex);
    YAML::Node node;
    node["type"]="StdoutLogAppender";
    if(m_level!=LogLevel::UNKNOW){
        node["level"]=LogLevel::ToString(m_level);
    }
    //
    if(m_formatter&&m_hasFormatter){
        node["formatter"]=m_formatter->getPattern();
    }
    //
    std::stringstream ss;
    ss<<node;
    return ss.str();
}
//
FileLogAppender::FileLogAppender(const std::string&filename)
     :m_filename(filename){
         reopen();
     }
     //
bool FileLogAppender::reopen(){
	MutexType::Lock lock(m_mutex);
    if(m_filestream){
        m_filestream.close();
    }
	//每一次打开时如果没文件就创建新文件，如果文件存在就以追加的方式写入
	m_filestream.open(m_filename,std::ios::app|std::ios::out);
    return !!m_filestream; 
}
void FileLogAppender::log(std::shared_ptr<Logger>logger,LogLevel::Level level,LogEvent::ptr event){
   if(level>=m_level){
	   //这里我们要实现的功能是每隔3s打开一次，方式文件在中途被删除
	   uint64_t now=event->getTime();
	   if(now>=(m_lastTime+3)){
		   reopen();
		   m_lastTime=now;
	   }
	   MutexType::Lock lock(m_mutex);
       m_filestream<<m_formatter->format(logger,level,event);
   }
}
std::string FileLogAppender::toYamlString(){
	MutexType::Lock lock(m_mutex);
    YAML::Node node;
    node["type"]="FileLogAppender";
    node["file"]=m_filename;
    if(m_level!=LogLevel::UNKNOW){
        node["level"]=LogLevel::ToString(m_level);
    }
    if(m_formatter&&m_hasFormatter){
        node["formatter"]=m_formatter->getPattern();
    }
    std::stringstream ss;
    ss<<node;
    return ss.str();
}
//
LogFormatter::LogFormatter(const std::string&pattern)
   :m_pattern(pattern){ init();}

std::string LogFormatter::format(std::shared_ptr<Logger>logger,LogLevel::Level level,LogEvent::ptr event){
    std::stringstream ss;
    //stringstream是ostream的子类的子类，相当于是孙子类
    for(auto&i : m_items){
        i->format(ss,logger,level,event);
    }
    return ss.str();
}
//该函数就是用来解析日志的格式
//     %xxx
//     %xxx{xxx} 
//     %%               
// 一共是这三种格式
//例如，默认的日志格式为
//       %d [%p] %f %l %m %n 
//      
void LogFormatter::init(){
   //分别是str  format   type
   std::vector<std::tuple<std::string,std::string,int>>vec;
   std::string nstr;
   for(size_t i=0;i<m_pattern.size();i++){
       if(m_pattern[i]!='%'){
           //在当前的nstr后面添加一个字符
           nstr.append(1,m_pattern[i]);
           continue;
       }
       //第三种格式，也就是%%的情况，表示转义
       if((i+1)<m_pattern.size()){
           if(m_pattern[i+1]=='%'){
               //%%连在一起的情况
               nstr.append(1,'%');
               continue;
           }
       }

       //
       size_t n=i+1;
       int fmt_status=0;//类似于状态机，用来对%xxx{xxx}进行解析
       size_t fmt_begin=0;

       std::string str;//用来记录解析到的str
       std::string fmt;//解析到的fmt
       while(n < m_pattern.size()) {
           //解析到了开头的xxx
            if(!fmt_status && (!isalpha(m_pattern[n]) && m_pattern[n] != '{'
                    && m_pattern[n] != '}')) {
                str = m_pattern.substr(i + 1, n - i - 1);
                break;
            }

            if(fmt_status == 0) {
                if(m_pattern[n] == '{') {
                    str = m_pattern.substr(i + 1, n - i - 1);
                    //std::cout << "*" << str << std::endl;
                    fmt_status = 1; //解析格式
                    fmt_begin = n;
                    ++n;
                    continue;
                }
            } else if(fmt_status == 1) {//对第二种格式内部的{xxx}进行解析
                if(m_pattern[n] == '}') {
                    fmt = m_pattern.substr(fmt_begin + 1, n - fmt_begin - 1);
                    //std::cout << "#" << fmt << std::endl;
                    fmt_status = 0;
                    ++n;
                    break;
                }
            }
            ++n;
            //第一种%xxx的格式
            if(n == m_pattern.size()) {
                if(str.empty()) {
                    str = m_pattern.substr(i + 1);
                }
            }
        }
        //解析到的就是第一种格式
        if(fmt_status == 0) {
            if(!nstr.empty()) {
                vec.push_back(std::make_tuple(nstr, std::string(), 0));
                nstr.clear();
            }
            vec.push_back(std::make_tuple(str, fmt, 1));
            i = n - 1;
        } else if(fmt_status == 1) {
            std::cout << "pattern parse error: " << m_pattern << " - " << m_pattern.substr(i) << std::endl;
            m_error = true;
            vec.push_back(std::make_tuple("<<pattern_error>>", fmt, 0));
        }
    }

    if(!nstr.empty()) {
        vec.push_back(std::make_tuple(nstr, "", 0));
    }
    static std::map<std::string, std::function<FormatItem::ptr(const std::string& str)> > s_format_items = {
#define XX(str, C) \
        {#str, [](const std::string& fmt) { return FormatItem::ptr(new C(fmt));}}
        //%d{%Y-%m-%d %H:%M:%S}%T%t%T%N%T%F%T[%p]%T[%c]%T%f:%l%T%m%n
        // %d %t %F [%p] %f %l %m %n 
        XX(m, MessageFormatItem),           //m:消息
        XX(p, LevelFormatItem),             //p:日志级别
        XX(r, ElapseFormatItem),            //r:累计毫秒数
        XX(c, NameFormatItem),              //c:日志名称
        XX(t, ThreadIdFormatItem),          //t:线程id
        XX(n, NewLineFormatItem),           //n:换行
        XX(d, DateTimeFormatItem),          //d:时间
        XX(f, FilenameFormatItem),          //f:文件名
        XX(l, LineFormatItem),              //l:行号
        XX(T, TabFormatItem),               //T:Tab
        XX(F, FiberIdFormatItem),           //F:协程id
        XX(N, ThreadNameFormatItem),        //N:线程名称
#undef XX
    };

    for(auto& i : vec) {
        if(std::get<2>(i) == 0) {                  //0代表的是string类型
            m_items.push_back(FormatItem::ptr(new StringFormatItem(std::get<0>(i))));
        } else {
            auto it = s_format_items.find(std::get<0>(i));
            if(it == s_format_items.end()) {
                m_items.push_back(FormatItem::ptr(new StringFormatItem("<<error_format %" + std::get<0>(i) + ">>")));
                m_error = true;
            } else {
                m_items.push_back(it->second(std::get<1>(i)));
            }
        }

        //std::cout << "(" << std::get<0>(i) << ") - (" << std::get<1>(i) << ") - (" << std::get<2>(i) << ")" << std::endl;
    }
    //std::cout << m_items.size() << std::endl;
  }
  //
LoggerManager::LoggerManager(){
    m_root.reset(new Logger());
    //默认携带一个输出屏幕
    m_root->addAppender(LogAppender::ptr (new StdoutLogAppender()));
    //
    m_loggers[m_root->getName()]=m_root;
    init();
}
Logger::ptr LoggerManager::getLogger(const std::string&name){
	MutexType::Lock lock(m_mutex);
    auto it=m_loggers.find(name);
    if(it!=m_loggers.end()){
        return it->second;
    }
    //如果不存在的话
    Logger::ptr logger(new Logger(name));
    logger->m_root=this->m_root;
    m_loggers[name]=logger;
    return logger;
}
struct LogAppenderDefine{
    int type=0;//2表示Stdout，1表示file
    LogLevel::Level level=LogLevel::UNKNOW;
    std::string formatter;
    std::string file;
    bool operator== (const LogAppenderDefine&rhv)const {
        return type==rhv.type
            && level==rhv.level
            && formatter==rhv.formatter
            && file==rhv.file;
    }
};
//每一个代表logs中的一项
struct LogDefine{
   std::string name;
   LogLevel::Level level=LogLevel::UNKNOW;
   std::string formatter;
   std::vector<LogAppenderDefine>appenders;
   bool operator== (const LogDefine&rhv)const {
       return name==rhv.name
          &&  level==rhv.level
          &&  formatter==rhv.formatter
          &&  appenders==rhv.appenders;
   }
   //由于要使用set容器来存储，所以需要对<进行重载
   bool operator< (const LogDefine&rhv)const{
       return name<rhv.name;
   }
};
//对set<LogDefine>这个自定义类型进行偏特化
template<>
class LexicalCast<std::string,LogDefine>{
public:
      LogDefine operator()(const std::string&str){
          YAML::Node node=YAML::Load(str);
          LogDefine ld;
          if(!node["name"].IsDefined()){
              //如果没有定义name，格式就是错误的
              std::cout<<"log config error:name is null,"<<node
              <<std::endl;
              throw std::logic_error("log config name is null");
          }
          ld.name=node["name"].as<std::string>();
          ld.level=LogLevel::FromString(node["level"].IsDefined()?node["level"].as<std::string>():"");
          if(node["formatter"].IsDefined()){
              ld.formatter=node["formatter"].as<std::string>();
          }
          //
          //下面就是appender
          if(node["appender"].IsDefined()){
              for(size_t i=0;i<node["appender"].size();++i){
                  auto x=node["appender"][i];
                  if(!x["type"].IsDefined()){
                      std::cout<<"log conf error:appender type is null,"<<x
                      <<std::endl;
                      continue;
                  }
                  std::string type=x["type"].as<std::string>();
                  LogAppenderDefine lad;
                  if(type=="FileLogAppender"){
                     lad.type=1;
                     if(!x["file"].IsDefined()){
                         std::cout << "log config error: fileappender file is null, " << x
                              << std::endl;
                              continue;
                     }
                     lad.file=x["file"].as<std::string>();
                     if(x["formatter"].IsDefined()){
                       lad.formatter = x["formatter"].as<std::string>();
                     }
                  }else if(type=="StdoutLogAppender"){
                       lad.type=2;
                       if(x["formatter"].IsDefined()) {
                        lad.formatter = x["formatter"].as<std::string>();
                      }
                  }else{
                       std::cout << "log config error: appender type is invalid, " << x
                              << std::endl;
                    continue;

                  }
                  ld.appenders.push_back(lad);
              }
          }
          return ld;
      }
};
template<>
class LexicalCast<LogDefine,std::string>{
public:
     std::string operator()(const LogDefine&ld){
         //这个的基本思路就是先将这个ld转化为node，然后通过流的方式返回
         YAML::Node node;
         node["name"]=ld.name;
         if(ld.level!=LogLevel::UNKNOW){
             node["level"]=LogLevel::ToString(ld.level);
         }
         if(!ld.formatter.empty()){
             node["formatter"]=ld.formatter;
         }
         //接下来是appender
         for(auto &i:ld.appenders){
             YAML::Node a;
             if(i.type==1){
                a["type"]="FileLogAppender";
                a["file"]=i.file;
             }else if(i.type==2){
                a["type"]="StdoutLogAppender";
             }
             //
             if(i.level!=LogLevel::UNKNOW){
                 a["level"]=LogLevel::ToString(i.level);
             }
             if(!i.formatter.empty()){
                 a["formatter"]=i.formatter;
             }
             node["appender"].push_back(a);
         }
         std::stringstream ss;
         ss<<node;
         return ss.str();
     }
};
JKYi::ConfigVar<std::set<LogDefine>>::ptr g_log_defines = JKYi::Config::Lookup("logs",std::set<LogDefine>(),"logs config");

//然后这里的话，通过全局对象在main函数执行之前进行初始化的特性，在它的构造函数中进行回调函数的注册
//通过下面这个函数，就是实现了在配置文件中修改日志输出
struct LogIniter{
    LogIniter(){
       g_log_defines->addListener([](const std::set<LogDefine>&old_value,const std::set<LogDefine>&new_value){
            //三种事件  新增、删除、修改
            JKYI_LOG_INFO(JKYI_LOG_ROOT())<<"on_logger_conf_changed";
            for(auto &i:new_value){
                auto it=old_value.find(i);
                JKYi::Logger::ptr logger;
                if(it==old_value.end()){
                    //旧得中不存在，属于是新增得
                    //logger.reset(new JKYi::Logger(i.name));
                    logger=JKYI_LOG_NAME(i.name);
                }else{
                    //存在
                    if(!(i==*it)){
                        //修改
                        logger=JKYI_LOG_NAME(i.name);
                    }
                }
                logger->setLevel(i.level);
                if(!i.formatter.empty()){
                    logger->setFormatter(i.formatter);
                }

                logger->clearAppender();
                for(auto&a:i.appenders){
                    JKYi::LogAppender::ptr ap;
                    if(a.type==1){
                       //输出到文件
                       ap.reset(new JKYi::FileLogAppender(a.file));
                    }else if(a.type==2){
                        ap.reset(new JKYi::StdoutLogAppender());
                    }
                    ap->setLevel(a.level);
                    //这里还需耀解决appender的formatter
                    if(!a.formatter.empty()){
                        LogFormatter::ptr formatter(new LogFormatter(a.formatter));
                        if(!formatter->isError()){
                            ap->setFormatter(formatter);
                        }else{
                            std::cout << "log.name=" << i.name << " appender type=" << a.type
                                      << " formatter=" << a.formatter << " is invalid" << std::endl;
                        }
                    }
                    logger->addAppender(ap);
                }
            }
            //删除的情况
            for(auto&i:old_value){
                auto it=new_value.find(i);
                if(it==new_value.end()){
                    //删除
                    JKYi::Logger::ptr logger=JKYI_LOG_NAME(i.name);
                    //这里的删除是逻辑上的删除，也就是将这个logger的级别调到最高
                    //也就是让用户使用不了
                    logger->setLevel((LogLevel::Level)100);
                    logger->clearAppender();
                }
                
            }
       });
    }
};
static LogIniter _log_init;//全局对象
void LoggerManager::init(){

}
std::string LoggerManager::toYamlString(){
	MutexType::Lock lock(m_mutex);
    YAML::Node node;
    for(auto&i :m_loggers){
        node.push_back(YAML::Load(i.second->toYamlString()));
    }
    std::stringstream ss;
    ss<<node;
    return ss.str();
}


}
