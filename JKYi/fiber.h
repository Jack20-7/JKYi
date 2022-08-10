#ifndef _JKYI_FIBER_H_
#define _JKYI_FIBER_H_

#include<memory>
#include<functional>
#include<ucontext.h>

namespace JKYi{

//封装的一个协程类
//我们的这个协程模型比较简单，他只允许主协程来切换协程执行，子协程不允许进行切换.子协程在执行完了之后都会返回到主协程去执行

class Scheduler;
class Fiber:public std::enable_shared_from_this<Fiber>{
friend class Scheduler;
public:
   typedef std::shared_ptr<Fiber> ptr;
   //协程的状态
   enum State{
      INIT,   //创建态  
	  HOLD,  //挂起态
	  EXEC,  //运行态
	  TERM,  //结束态
	  READY, //就绪态
	  EXCEPT//异常态
   };
public:
   //线程的主协程就是通过默认构造来创建的
    Fiber();

	//析构函数
	~Fiber();

	//有参构造，执行的协程就是通过有参构造来进行创建
	//这里的use_caller表示的是为调度器的主调度协程
	Fiber(std::function<void ()>cb,size_t stacksize = 0,bool use_caller = false);

	//当一个协程执行完成后，可以通过调用该函数来让他去执行别的函数，重复利用他的资源
	void reset(std::function<void ()>cb);
	//令主协程挂起，去执行当前这个协程
	void swapIn();
	//挂起当前的这个协程，然后去执行主协程
	void swapOut();
	//执行当前协程
	void call();
	//挂起当前协程
	void back();
	//返回当前协程的id
	uint64_t getId()const {return m_id;}
	//返回当前协程的状态
	State getState()const{return m_state;}
public:
   //static成员函数，用于通过类名直接进行调用

   //将传入的协程设置为当前正在执行的协程
   static void SetThis(Fiber*f);
   //
   static Fiber::ptr GetThis();
   //将当前正在执行的协程设置为就绪态
   static void YieldToReady();
   //
   static void YieldToHold();
   //返回当前的协程总数
   static uint64_t TotalFibers();
   //
   static void MainFunc();
   //
   static void CallerMainFunc();
   //返回当前协程的id
   static uint64_t GetFiberId();
private:
   //协程id、
   uint64_t m_id = 0;
   //协程运行栈的大小
   uint32_t m_stacksize = 0;
   //协程的栈
   void *m_stack = nullptr;
   //协程的状态
   State m_state = INIT;
   //协程的上下文(CPU上下文，栈)的相关信息
   ucontext_t m_ctx;
   //协程的执行函数
   std::function<void ()>m_cb;
   
};
}
#endif
