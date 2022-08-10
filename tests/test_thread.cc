#include"../JKYi/JKYi.h"
#include<unistd.h>
//普通的测试使用root打日志就行了
JKYi::Logger::ptr g_logger=JKYI_LOG_ROOT();
int count=0;
JKYi::RWMutex mutex; 
void fun(){
  JKYI_LOG_INFO(g_logger)<<"name: "<<JKYi::Thread::GetName()
                         <<" this.name: "<<JKYi::Thread::GetThis()->getName()
                         <<" id: "<<JKYi::GetThreadId()
                         <<" this.id: "<<JKYi::Thread::GetThis()->getId();
		for(int i=0;i<10000000;++i){
			JKYi::RWMutex::WriteLock lock(mutex);
			count++;
		}
	JKYI_LOG_INFO(g_logger)<<"thread:"<<JKYi::Thread::GetThis()->getId()<<"end";
}
int main(int argc,char ** argv){
   JKYI_LOG_INFO(g_logger)<<"thread test begin";
   //类似于线程池
   std::vector<JKYi::Thread::ptr>thrs;
   for(int i=0;i<5;++i){
     JKYi::Thread::ptr thr(new JKYi::Thread(&fun,"name_"+std::to_string(i))); 
     thrs.push_back(thr);
   }
   for(int i=0;i<5;++i){
      thrs[i]->join(); 
   }
   JKYI_LOG_INFO(g_logger)<<count;
   JKYI_LOG_INFO(g_logger)<<"thread test end";
   return 0;
}

