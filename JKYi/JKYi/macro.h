#ifndef _JKTI_MACRO_H_
#define _JKYI_MACRO_H_

//该头文件用来对常用的宏进行封装

#include<string>
#include<assert.h>
#include"log.h"
#include"util.h"

//对断言的封装
#define JKYI_ASSERT(x) \
{ \
   if(!(x)){ \
      JKYI_LOG_ERROR(JKYI_LOG_ROOT())<<"ASSERTION:"#x \
	  <<"\nbacktrace:\n" \
	  <<JKYi::BacktraceToString(100,2,"  ");\
	  assert(x); \
   } \
}
//携带额外信息的宏定义的封装
#define JKYI_ASSERT2(x,w) \
{ \
    if(!(x)){ \
		JKYI_LOG_ERROR(JKYI_LOG_ROOT())<<"ASSERTION:"#x \
		<<"\n"<<w \
		<<"\nbacktrace\n" \
		<<JKYi::BacktraceToString(100,2,"  "); \
		assert(x); \
	} \
}
#endif
