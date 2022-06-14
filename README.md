# JKYi
高性能的C++服务器框架

## 1.日志模块+配置系统

对于JKYi的日志，我们可以在log.yml这个配置文件中对日志的输出方式进行配置，配置的默认格式为
```cpp
    - name: root
      level: info
      formatter: '%d%T%m%n'
      appender:
          - type: FileLogAppender
            file: root.txt
          - type: StdoutLogAppender
```

如果没有配置的话，默认的格式就是以上这种，使用的方式就是
```cpp
JKYI_LOG_INFO(JKYI_LOG_ROOT())<<"xxxxx"
JKYI_LOG_DEBUG(JKYI_LOG_ROOT())<<"xxxxx"
JKYI_LOG_ERROR(JKYI_LOG_ROOT())<<"xxxxx"
JKYI_LOG_FATAL(JKYI_LOG_ROOT())<<"xxxxx"
JKYI_LOG_WARN(JKYI_LOG_ROOT())<<"xxxxx"
```

通过上诉这种方式来输出不同日志级别的日志，但是要注意的是当前的配置文件中的日志级别是info，只有级别>=info的级别才会被输出。

对于日志的输出格式，定义的方式为
```cpp
m:日志消息体
p:日志级别
r:累计毫秒数
c:日志名称
t:线程id
n:换行
d:时间
f:文件名
l:行号
T:Tab
F:协程id
```
他们之间通过%号的形式进行间隔，用户可以通过修改配置文件中的formatter的方式来自定义输出的日志格式

配置文件中的appender表示的是日志的输出地,StdoutLogAppender表示的是之间输出到显示屏，FileLogAppender表示的是输出到日志文件，文件名通过fiie来指定

举例用法:

我们可以现在配置文件中写上我们自定义的格式，如:
```cpp
    - name: system
      level: debug
      formatter: '%d%T%m%n'
      appender:
          - type: FileLogAppender
            file: system.txt
            formatter: '%d%T[%p]%T%m%T%n'
          - type: StdoutLogAppender
```

加上之后，使用的方式就是
```cpp
JKYi::Logger:: g_system_log=JKYI_LOG_NAME("system");
//这里如果直接使用的话，由于配置文件还没有被加载，实际使用的还是root的格式
JKYI_LOG_INFO(g_system_log)<<"XXXX"

//然后如果我们想要使用我们自己新定义的配置的话，需要通过YAML来进行加载，也就是
YAML::Node& node=YAML::LoadFile("配置文件的路径");
JKYi::Config::LoadFromYaml(node)
//以上操作完成之后，如果在使用g_system_log打日志的话，就是根据我们用户自定义的配置
JKYI_LOG_INFO(g_system_log)<<"xxx"
//这样的话，输出到屏幕的日志格式就是'%d%T%m%n'，写入到system.txt的日志格式为'%d%T[%p]%T%m%T%n'
```

## 2.线程模块

该服务器框架内的主要的工作是基于协程来完成的，线程模块的话封装的比较简单，因为线程仅仅是作为协程的容器而存在的.

线程模块一共包含:封装好的线程、用于实现线程同步的互斥量、信号量、自旋锁、读写锁、原子锁.框架内的日志模块和配置模块都默认使用自旋锁实现了线程安全.

线程的使用方法是

```cpp
JKYi::RWMutex m_mutex;
int count=0;
void fun(){
 for(int i=0;i<10000;++i){
 JKYi::RWMutex::WriteLock lock(&m_mutex);
   count++; 
 }
 return ;
}
int main(int argc,char** argv){
  std::vector<JKYi::Thread::ptr> v;
   for(int i=0;i<3;++i){
     JKYi::Thread::ptr thr(new JKYi::Thead(&fun,"thread "+std::to_string(i)));
     v.push_back(thr);
  }
  for(size_t i=0;i<v.size();++i){
     v[i].join();
  }

  return 0;
}
```
## 3.协程模块

本服务器的主要的业务就是就是通过协程来完成，协程我们可以看作是轻量级的线程，他拥有自己独立的CPU上下文以及栈。相比于线程而言，协程之间的切换是在用户态来完成的，不需要陷入内核态，所以协程间的切换带来的开销是远小于线程的。并且协程的调度是完全由我们用户来掌控的，相当于是同步的，所以也没有线程的异步所带来的问题。

本服务器框架的协程模型是比较简单的.每一个线程中都拥有一个主协程,然后整个的逻辑是由子协程来完成的，并且协程的切换只能够由主协程来完成，子协程不能够切换切换到其他的子协程，子协程在执行完成之后就会返回到主协程.


```cpp
void func1(){
	JKYI_LOG_INFO(JKYI_LOG_ROOT())<<"func1 start";
	//切换回主协程执行
	JKYi::Fiber::YieldToHold();
	//
	JKYI_LOG_INFO(JKYI_LOG_ROOT())<<"func1 end";
	// 
	JKYi::Fiber::yieldToHold();
}

int main(int argc,char**argv){
	//这里面当发现当前没有协程创建时就会自动创建一个子协程
	JKYi::Fiber::GetThis();
     //创建出一个子协程，在创建子协程时，我们可以通过构造函数的第二个参数给他指定栈的大小，如果没有指定的话，默认使用的配置系统中记录的大小，也就是1M 
	JKYi::ptr fiber(new JKYi::Fiber(&func1));
	//
	JKYI_LOG_INFO(JKYI_LOG_ROOT())<<"main fiber start";
     //切换到子协程去执行	
	fiber->swapIn();
	//
	JKYI_LOG_INFO(JKYI_LOG_ROOT())<<"main fiber swapIn after";
	
	fiber->swapIn();
	//
	JKYI_LOG_INFO(JKYI_LOG_ROOT())<<"main fiber end";
	//
	fiber->swapIn();

	return 0;
}
```
上面这个程序，主协程最后一次swapIn到子协程之后，子协程执行完毕，然后就会返回到主协程，然后程序结束。最终主协程和子协程都能够被销毁



