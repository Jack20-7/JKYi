#ifndef _JKYI_SINGLETON_H_
#define _JKYI_SINGLETON_H_
#include<memory>

namespace JKYi{
//封装一个单例类
//T表示的是类型，X表示的是创造多个实例对应的的tag  N表示的是同一个tag创建多个实例的索引
template<class T,class X=void,int N=0>
class Singleton{
public:
   static T* GetInstance(){
       static T t;
       return &t;
   }
};
//这个是单例模式的智能指针类
template<class T,class X=void,int N=0>
class SingletonPtr{
public:
    static std::shared_ptr<T> getInstance(){
        static std::shared_ptr<T> t(new T());
        return t;
    }
};


}


#endif