#ifndef _JKYI_NET_SINGLETON_H_
#define _JKYI_NET_SINGLETON_H_

#include"JKYi/noncopyable.h"

#include<assert.h>
#include<pthread.h>
#include<stdlib.h>

namespace JKYi{
namespace net{

//单例类
//不考虑对象的销毁，因为在短期运行的服务器上程序推出自然而然就销毁了，而长期运行的服务器上的话，程序又不需要考虑正常推出
template<class T>
class Singleton : public Noncopyable{
public:
    Singleton() = delete;
    ~Singleton() = delete;

    static T& instance(){
        pthread_once(&ponce_,&Singleton::init);
        assert(value_ != NULL);
        return *value_;
    }
private:
    static void init(){
        value_ = new T();
    }
private:
    static pthread_once_t ponce_;
    static T*             value_;
};

template<class T>
pthread_once_t Singleton<T>::ponce_ = PTHREAD_ONCE_INIT;

template<class T>
T* Singleton<T>::value_ = NULL;


}//namespace net
}//namespace JKYi

#endif
