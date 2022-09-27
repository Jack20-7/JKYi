#ifndef _JKYI_TYPES_H_
#define _JKYI_TYPES_H_

#include"JKYi/macro.h"

#include<stdint.h>
#include<string.h>
#include<string>

namespace JKYi{

//专门用来完成隐式类型转换，如果不符合条件在编译期就会报错
template<typename To,typename From>
inline To implicit_cast(From const & f){
    return f;
}

//专门用来完成向下类型转化，也就是基类->子类的转换
template<typename To,typename From>
inline To down_cast(From* f){
  // Ensures that To is a sub-type of From *.  This test is here only
  // for compile-time type checking, and has no overhead in an
  // optimized build at run-time, as it will be optimized away
  // completely.
  if (false)
  {
    implicit_cast<From*, To>(0);
  }

#if !defined(NDEBUG) && !defined(GOOGLE_PROTOBUF_NO_RTTI)
  assert(f == NULL || dynamic_cast<To>(f) != NULL);  // RTTI: debug mode only!
#endif
  return static_cast<To>(f);
}

}


#endif
