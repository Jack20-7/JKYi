#include"JKYi/reactor/Singleton.h"
#include"JKYi/reactor/CurrentThread.h"
#include"JKYi/reactor/Thread.h"

#include<stdio.h>
#include<string>

class Test{
public:
    Test(){
      printf("tid = %d,constructing %p\n",JKYi::CurrentThread::tid(),this);
    }
    ~Test(){
      printf("tid = %d,destructing %p %s\n",JKYi::CurrentThread::tid(),this,name_.c_str());
    }
    const std::string& name()const { return name_; }
    void setName(const std::string& name) { name_ = name; }
private:
    std::string name_;
};

void threadFunc(){
    printf("tid = %d,%p name = %s\n",
              JKYi::CurrentThread::tid(),
             &JKYi::net::Singleton<Test>::instance(),
              JKYi::net::Singleton<Test>::instance().name().c_str());
    JKYi::net::Singleton<Test>::instance().setName("only one,changed");
}

int main(void){
    JKYi::net::Singleton<Test>::instance().setName("only once");
    JKYi::net::Thread t1(threadFunc);
    t1.start();
    t1.join();

    printf("tid = %d,%p name = %s\n",
             JKYi::CurrentThread::tid(),
             &JKYi::net::Singleton<Test>::instance(),
             JKYi::net::Singleton<Test>::instance().name().c_str());
    return 0;
}
