#ifndef _JKTI_MACRO_H_
#define _JKYI_MACRO_H_

//该头文件用来对常用的宏进行封装

#include<string>
#include<assert.h>
#include"log.h"
#include"util.h"

//对编译器判断的优化
#if defined _GNUC_ || defined _llvm_
//如果是以上的两种编译器的话
//告诉编译器以下条件大概率成立
# define JKYI_LIKELY(x)   _builtin_expect(!!(x),1)
# define JKYI_UNLIKELY(x) _builtin_expect(!!(x),0)
#else
//如果编译器不支持的话，就和以前一样
# define JKYI_LIKELY(x)   (x)
# define JKYI_UNLIKELY(x) (x)
#endif

//对断言的封装
#define JKYI_ASSERT(x) \
{ \
   if(JKYI_UNLIKELY(!(x))){ \
      JKYI_LOG_ERROR(JKYI_LOG_ROOT())<<"ASSERTION:"#x \
	  <<"\nbacktrace:\n" \
	  <<JKYi::BacktraceToString(100,2,"  ");\
	  assert(x); \
   } \
}

//携带额外信息的宏定义的封装
#define JKYI_ASSERT2(x,w) \
{ \
    if(JKYI_UNLIKELY(!(x))){ \
		JKYI_LOG_ERROR(JKYI_LOG_ROOT())<<"ASSERTION:"#x \
		<<"\n"<<w \
		<<"\nbacktrace\n" \
		<<JKYi::BacktraceToString(100,2,"  "); \
		assert(x); \
	} \
}
#endif
