#ifndef _JKYI_NONCOPYABLE_H_
#define _JKYI_NONCOPYABLE_H_
//该类用来修饰那些不能够被拷贝的类
namespace JKYi{
class Noncopyable{
public:
  //默认构造
  Noncopyable()=default; 
  //拷贝构造
  Noncopyable(const Noncopyable&)=delete;
  //拷贝赋值
  Noncopyable& operator= (const Noncopyable&)=delete;
  //移动构造
  Noncopyable(Noncopyable&&)=delete;
  //移动赋值
  Noncopyable&& operator= (Noncopyable&&)=delete;
  //析构函数
  ~Noncopyable()=default;
};
}
#endif
