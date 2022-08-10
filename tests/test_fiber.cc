#include"/home/admin/workSpace/JKYi/JKYi.h"

JKYi::Logger::ptr g_logger=JKYI_LOG_ROOT();
void func1(){
  JKYI_LOG_INFO(g_logger)<<"func1 start"; 
  JKYi::Fiber::YieldToHold(); 
  JKYI_LOG_INFO(g_logger)<<"fun1 end";
  JKYi::Fiber::YieldToHold();

}
void test_fiber(){
   JKYi::Fiber::GetThis();
   //
   JKYI_LOG_INFO(g_logger)<<"main fiber begin";
   JKYi::Fiber::ptr fiber(new JKYi::Fiber(func1));
   fiber->swapIn();
   JKYI_LOG_INFO(g_logger)<<"fiber swapIn after";
   fiber->swapIn();
   JKYI_LOG_INFO(g_logger)<<"main fiber end";
   //
   fiber->swapIn();

}
int main(int argc,char**argv){
	JKYi::Thread::setName("main");

	std::vector<JKYi::Thread::ptr>thrs;
	for(int i=0;i<3;++i){
		thrs.push_back( JKYi::Thread::ptr(new JKYi::Thread(&test_fiber,"name_"+std::to_string(i))));
	}
	for(size_t i=0;i<thrs.size();++i){
		thrs[i]->join();
	}
	return 0;
}
