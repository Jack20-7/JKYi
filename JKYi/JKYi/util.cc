#include"util.h"
#include<execinfo.h>
#include<string>
#include<unistd.h>
#include"log.h"

namespace JKYi{

static JKYi::Logger::ptr g_logger=JKYI_LOG_NAME("system");
   //用来返回当前线程的id
pid_t GetThreadId(){
       return syscall(SYS_gettid);
}
   //返回当前协程的id
u_int32_t GetFiberId(){
       return 0;
}
   
void Backtrace(std::vector<std::string>&bt,int size,int skip){
	   void ** array=(void**)malloc(sizeof(void*)*size);
	   size_t s=::backtrace(array,size);
	   
	   char**strings=backtrace_symbols(array,s);
	   if(strings==nullptr){
          JKYI_LOG_ERROR(g_logger)<<"backtrace_symbols error";
		  return ;
	   }
	   for(size_t i=skip;i<s;++i){
		   bt.push_back(strings[i]);
	   }
	   free(array);
	   free(strings);
   }
std::string BacktraceToString(int size,int skip,const std::string&prefix){
      std::vector<std::string>bt;
	  Backtrace(bt,size,skip);
	  std::stringstream ss;
	  for(size_t i=0;i<bt.size();++i){
       ss<<prefix<<bt[i]<<std::endl;
	  }
	  return ss.str();
}
//获得当前时间对应的毫秒数
uint64_t GetCurrentMS(){
    struct timeval tv;
	gettimeofday(&tv,NULL);
	return tv.tv_sec*1000ul+tv.tv_usec/1000;
}
//获得当前的微秒数
uint64_t GetCurrentUS(){
	struct timeval tv;
	gettimeofday(&tv,NULL);
	return tv.tv_sec*1000*1000ul+tv.tv_usec;
}

}
