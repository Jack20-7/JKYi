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

## 协程调度模块

协程调度模主要是由scheduler这个类来实现的，实现的模型就是scheduler中多个线程，每一个线程中有多个协程.然后scheduler中还含有一个任务队列，我们在使用的时候就只需要将要处理的任务放到任务队列中去就可以让调度器中的线程里面的协程来对该任务进行处理.任务的类型的话可以是回调函数，也可以是协程.例如

```cpp
void tese_fiber(){
	static int s_count=5;
	
	if(--s_count>=0){
		JKYi::Scheduler::GetThis()->schedule(&test_fiber,JKYi::GetThreadId());
	}
	return ;
}
int main(void){

	//第一个参数表示调度器中管理的线程的数目
	//第二个参数表示是否将当前线程作为调度器中的调度线程来使用
	//第三个参数表示的就是线程的名称
	JKYi::Scheduler sc(3,true,"test");

	//对调度器中的线程进行创建
	sc.start();

	//向调度器中添加任务来处理
	//该函数就是用来向调度器中添加任务的，添加的任务可以是回调函数，也可以是
	//协程并且可以通过在第二个参数来让该任务在特定的线程中来执行
	sc.schedule(&test_fiber);

	sc.stop();

	return 0;
}

```

## 定时器模块

通过定时器来对需要在特定时间进行处理的任务进行处理.添加的定时器可以是普通的定时器，也可以是条件定时器,支持一次性触发、循环触发等.这个定时器一般不会单独来实现，他需要配合io协程调度一起使用.定时器的精度是毫秒级别的，这样做是为了配合epoll.

## IO协程调度模块

io协程调度模块其实就是基于上面的定时器模块和协程调度模块来实现的，我们平时使用就是直接使用io协程调度器就行了.io协程调度模块中通过对epoll的封装来对协程进行最大限度的复用，减少了资源的开销，极大的提高了程序执行的效率.支持对socket的的读写事件处理

示例如下:

``` cpp

//下面测试对socket的支持
void test_socket(){

	sock=socket(AF_INET,SOCK_STREAM,0);
	fcntl(sock,F_SETFL,O_NONBLOCK);
	//
	struct sockaddr_in addr;
	memset(&addr,0,sizeof(addr));
	addr.sin_family=AF_INET;
	addr.sin_port=htons(80);
	inet_pton(AF_INET, "220.181.38.148", &addr.sin_addr.s_addr);
	if(!connect(sock,(sockaddr*)&addr,sizeof(addr))){

	}else if(errno==EINPROGRESS){
	   JKYI_LOG_INFO(g_logger)<<"add event errno="<<errno<<"  "<<strerror(errno);
	   //田间对socket写事件的关注并注册对应的回调函数
	   //第一个参数是socket
	   //第二个参数是关注的事件
	   //第三个参数是触发事件时的回调函数
	   JKYi::IOManager::GetThis()->addEvent(sock,JKYi::IOManager::READ,[](){
		   JKYI_LOG_INFO(g_logger)<<"read callback";
	   });

	   JKYi::IOManager::GetThis()->addEvent(sock,JKYi::IOManager::WRITE,[](){
		   JKYI_LOG_INFO(g_logger)<<"write callback";
		   //
		   //取消对该事件的关注并且强制触发对应的回调函数
		   JKYi::IOManager::GetThis()->cancelEvent(sock,JKYi::IOManager::READ);
		   close(sock);
	   });
	  }else{
		JKYI_LOG_INFO(g_logger)<<"connectiong is establish";
	}


}

//下面是对定时器的测试
JKYi::Timer::ptr s_timer;
void test_timer(){
  //在调度器中开两个线程
  JKYi::IOManager iom(2);

  //添加定时器
  //第一个参数是定时器的超时时间，毫秒级别的
  //第二个参数就是回调函数，如果未指定的话就默认使用当前的协程来处理
  //第三个参数是是否需要循环触发
  iom.addTimer(1000,[](){
   static int i=0;
   if(++i==3){
     //取消定时器
     s_timer->cancel();
   }
  },true);
}
int main(void){
	test_timer();
	test_socket();
	return 0;
}

```

## hook模块

hook模块主要是对socket的一些函数进行hook，比如socket、connect、accept等。通过对他们进行hook的方式将他们从同步操作转化为异步操作，可以极大的提高程序执行的效率并且可以简化之后网络模块的编写


## 网络模块的添加

### 地址模块

JKYi中分别对IPv4、IPv6、Uniux它们对应的socekt地址进行的封装，这样我们使用起来更加的方面，方便了接下来网络模块的一个编写，该模块的结构是


```
     ------Address------
		 -        -
		-          -
	   -            -
	 IPAddress    UnixAddress
	  -     -
	 -       -
	-         -
 IPv4Address   IPv6Address


```

然后具体的功能的话支持根据域名返回地址，查询当前机器的网卡,查子网掩码、广播地址等

### socket模块的封装 

socket模块在封装的过程中就利用到了之前hook模块hook的那些网络编程相关的函数.通过对socket的封装可以让我们在进行网络编程的时候更加的方便,更加的高效.并且支持域名解析的功能,示例如下:


```

void test(){

    //根据域名获取IP
    JKYi::IPAddress::ptr addr=JKYi::Address::LookupAnyIPAddress("www.baidu.com");
    if(addr){
      //成功
    }else{
        //失败
    }

    //根据连接方的地址族地址创建TCPsocket
    JKYi::Socket::ptr sock=JKYi::Socket::CreateTCP(addr);
    addr->setPort(80);
    if(sock){
        //成功
    }else{
        //创建失败
    }

    if(!sock->connect(addr)){
        //建立连接失败
    }else{
        //建立连接成功
    }
    
    const char msg[]="GET / HTTP1.0\r\n\r\n";
    //向连接方发送消息
    int rt=sock->send(msg,sizeof(msg));
    if(rt<=0){
        //失败
    }
    std::string buff;
    buff.resize(4096);
    //接收连接方返回的信息
    rt=sock->recv(&buff[0],buff.size());
    if(rt<=0){
       //接收失败
    }else{
        //接收成功
    }

    buff.resize(rt);

    //将收到的消息打印出来
    std::cout<<buff<<std::endl;
}

```

## Bytearray模块

Bytearray(二进制序列化模块),用作网络编程中数据的载体，支持int8、uint8、int16、uint16、int32、uint32、int64、uint64等，并且还支持对int32、int64进行压缩，使用的压缩算法是protobuf的varint算法.支持对string的读写操作、字节序的转化、以及将序列化数据写入文件以及将数据从文件中读出进行反序列化


## HTTP模块

HTTP模块的话，对HTTP请求报文和HTTP响应报文进行的封装，分别封装成了HttpRequest和HttpResponse,并且对于HTTP报文的解析，是通过ragel来进行解析的，ragel其实就是有效状态机

