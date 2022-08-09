#include"fiber.h"
#include"config.h"
#include"macro.h"
#include"log.h"
#include"scheduler.h"
#include<atomic>

namespace JKYi{
 //服务器框架中的日志统一使用system
 static JKYi::Logger::ptr g_logger=JKYI_LOG_NAME("system");
 //记录分配的协程id
 static std::atomic<uint64_t>s_fiber_id{0};
 //记录创建的协程数目
 static std::atomic<uint64_t>s_fiber_count{0};

 //通过使用线程局部变量来记录当前正在执行的协程
 static thread_local Fiber* t_fiber = nullptr;

 //记录当前线程的主协程
 static thread_local Fiber::ptr t_threadFiber = nullptr;

 //用来实现通过配置系统修改协程栈的大小
 static ConfigVar<uint32_t>::ptr g_fiber_stack_size=JKYi::Config::Lookup<uint32_t>("fiber.stack_size",1024*1024,"fiber stack size");

//这里封装一个协程栈的分配器
class MallocStackAllocator{
public:
   static void* Alloc(size_t size){
	   return malloc(size);
   }
   static void Dealloc(void *ptr,size_t size){
	   return free(ptr);
   }
};
//
using StackAllocator=MallocStackAllocator;


Fiber::Fiber(){
   m_state=EXEC;
   SetThis(this);
   //将自己设置为主协程 
   //注意，shared_from_this不能够在构造函数中调用
   //t_threadFiber=shared_from_this();
   //将创建时的CPU上下文作为自己的上下文保存起来
   if(getcontext(&m_ctx)){
	   JKYI_ASSERT2(false,"getcontext");
   }
   ++s_fiber_count;

   //JKYI_LOG_DEBUG(g_logger)<<"main fiber is created";
}
//
Fiber::Fiber(std::function<void ()>cb,size_t stacksize,bool use_caller)
    :m_id(++s_fiber_id),
     m_cb(cb){
	++s_fiber_count;
	//如果用户传入了栈大小，就用用户指定的栈大小，如果没有的话，默认使用配置
	m_stacksize = stacksize ? stacksize : g_fiber_stack_size->getValue();
	m_stack = StackAllocator::Alloc(m_stacksize);
	//
	if(getcontext(&m_ctx)){
		JKYI_ASSERT2(false,"getcontext");
	}
	//设置本协程自己的上下文
	//ul_link里面设置的是写一个关联的协程的上下文
	m_ctx.uc_link=nullptr;
	m_ctx.uc_stack.ss_sp = m_stack;
	m_ctx.uc_stack.ss_size = m_stacksize;
   //
   if(!use_caller){
      makecontext(&m_ctx,&Fiber::MainFunc,0);
   }else{
	   //如果是跑在调度器中的主调度协程的话
	   makecontext(&m_ctx,&Fiber::CallerMainFunc,0);
   }
	//
	JKYI_LOG_DEBUG(g_logger)<<"subFiber is created,fiber_id="<<m_id;
}
//
Fiber::~Fiber(){
	--s_fiber_count;
    if(m_stack){
       //如果析构的是子协程
	   JKYI_ASSERT(m_state==TERM||m_state==EXCEPT||m_state==INIT);
	   StackAllocator::Dealloc(m_stack,m_stacksize);
	}else{
       //如果析构的是主协程的话
	   JKYI_ASSERT(!m_cb);
	   JKYI_ASSERT(m_state==EXEC);
	   Fiber*cur=t_fiber;
	   if(cur==this){
		   SetThis(nullptr);
	   }
	}
	//JKYI_LOG_DEBUG(g_logger)<<"Fiber::~Fiber id="<<m_id
	                       //<<" total="<<s_fiber_count;
}
//
void Fiber::reset(std::function<void()>cb){
    JKYI_ASSERT(m_stack);   
	JKYI_ASSERT(m_state==INIT||m_state==TERM||m_state==EXCEPT);
    
	m_cb=cb;
	if(getcontext(&m_ctx)){
		JKYI_ASSERT2(false,"getcontext");
	}
	//
	m_ctx.uc_link=nullptr;
	m_ctx.uc_stack.ss_sp=m_stack;
	m_ctx.uc_stack.ss_size=m_stacksize;

	makecontext(&m_ctx,&Fiber::MainFunc,0);
	m_state=INIT;
	return ;
}
//在主协程中使用，用来执行子协程
void Fiber::call(){
	SetThis(this);
	//
	m_state = EXEC;
	if(swapcontext(&t_threadFiber->m_ctx,&m_ctx)){
		JKYI_ASSERT2(false,"swapcontext");
	}
}
//在子协程中调用，用以返回主协程执行
void Fiber::back(){
	SetThis(t_threadFiber.get());
	//
	if(swapcontext(&m_ctx,&t_threadFiber->m_ctx)){
		JKYI_ASSERT2(false,"swapcontext");
	}
}
//同调度器的主调度协程进行切换
void Fiber::swapIn(){
    SetThis(this);
	JKYI_ASSERT(m_state != EXEC);
	//
	m_state = EXEC;
	if(swapcontext(&Scheduler::GetMainFiber()->m_ctx,&m_ctx)){
		JKYI_ASSERT2(false,"swapcontext");
	}
}
//
void Fiber::swapOut(){
	SetThis(Scheduler::GetMainFiber());
	//这里还是不要把状态设置为hold，因为在子协程执行完之后可能会出问题
	//m_state=HOLD;
	
	if(swapcontext(&m_ctx,&Scheduler::GetMainFiber()->m_ctx)){
		JKYI_ASSERT2(false,"swapcontext");
	}
    return ;
}
//
void Fiber::SetThis(Fiber*f){
	t_fiber = f;
}
//
Fiber::ptr Fiber::GetThis(){
	if(t_fiber){
		return t_fiber->shared_from_this();
	}
	//还未创建主协程
	Fiber::ptr main_fiber(new Fiber);
	JKYI_ASSERT(t_fiber == main_fiber.get());
	t_threadFiber = main_fiber;
	return t_fiber->shared_from_this();
}
//
void Fiber::YieldToReady(){
    Fiber::ptr cur=GetThis();
	JKYI_ASSERT(cur->m_state==EXEC);
	cur->m_state=READY;
    cur->swapOut();
}
//
void Fiber::YieldToHold(){
	Fiber::ptr cur = GetThis();
	JKYI_ASSERT(cur->m_state == EXEC);
	//cur->m_state=HOLD;
	cur->swapOut();
}
//
uint64_t Fiber::TotalFibers(){
	return s_fiber_count;
}
//
void Fiber::MainFunc(){
	Fiber::ptr cur = GetThis();
	JKYI_ASSERT(cur);
	//
	try{
	  cur->m_cb();
	  cur->m_cb = nullptr;
	  cur->m_state = TERM;
	}catch(std::exception& ex){
      cur->m_state = EXCEPT;
	  JKYI_LOG_ERROR(g_logger)<<"Fiber except:"<<ex.what()
	                          <<"fiber_id="<<cur->getId()
							  <<std::endl
							  <<JKYi::BacktraceToString();
	}catch(...){
       cur->m_state = EXCEPT;
	   JKYI_LOG_ERROR(g_logger)<<"Fiber except"
	                           <<"fiber_id="<<cur->getId()
							   <<std::endl
							   <<JKYi::BacktraceToString();
	}
	//如果是正常执行完的话，那么就切切换到主协程去执行
	auto raw_ptr = cur.get();
	cur.reset();
	//
	raw_ptr->swapOut();
	//
	JKYI_ASSERT2(false,"never reach fiber_id"+std::to_string(raw_ptr->getId()));
}
uint64_t Fiber::GetFiberId(){
	if(t_fiber){
		return t_fiber->getId();
	}
	return 0;
}
void Fiber::CallerMainFunc(){
	Fiber::ptr cur=GetThis();
	JKYI_ASSERT(cur);
	//
	try{
	  cur->m_cb();
	  cur->m_cb = nullptr;
	  cur->m_state = TERM;
	}catch(std::exception&ex){
      cur->m_state = EXCEPT;
	  JKYI_LOG_ERROR(g_logger)<<"Fiber except:"<<ex.what()
	                          <<"fiber_id="<<cur->getId()
							  <<std::endl
							  <<JKYi::BacktraceToString();
	}catch(...){
       cur->m_state = EXCEPT;
	   JKYI_LOG_ERROR(g_logger)<<"Fiber except"
	                           <<"fiber_id="<<cur->getId()
							   <<std::endl
							   <<JKYi::BacktraceToString();
	}
	//如果是正常执行完的话，那么就切切换到主协程去执行
	auto raw_ptr = cur.get();
	cur.reset();
	//
	raw_ptr->back();
	//
	JKYI_ASSERT2(false,"never reach fiber_id"+std::to_string(raw_ptr->getId()));
//
}

}






























