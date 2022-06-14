#include"../JKYi/JKYi.h"

static JKYi::Logger::ptr  g_logger=JKYI_LOG_ROOT();
void test_fiber(){
   static int s_count=5;
   JKYI_LOG_INFO(g_logger)<<"test int fiber,s_count="<<s_count;
   //sleep(1);
   //
   sleep(1);
   if(--s_count>=0){
	   JKYi::Scheduler::GetThis()->schedule(&test_fiber,JKYi::GetThreadId());
   }

   return ;
}
int main(int argc,char**argv){
	JKYI_LOG_INFO(g_logger)<<"mian";
	JKYi::Scheduler sc(3,true,"test");//(3,true,"test");
	sc.start();
	//
	//sleep(2);
	JKYI_LOG_INFO(g_logger)<<"schedule";
	sc.schedule(&test_fiber);

	sc.stop();
	JKYI_LOG_INFO(g_logger)<<"over";
	return 0;
}
