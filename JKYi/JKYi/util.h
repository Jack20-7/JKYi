#ifndef _JKYI_UTIL_H_
#define _JKYI_UTIL_H_

#include<pthread.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/syscall.h>
#include<stdio.h>
#include<stdint.h>
//该文件中定义的是一些常用的函数
namespace JKYi{
    //获取线程id的函数
   pid_t GetThreadId();  
   //返回当前协程的id
   u_int32_t GetFiberId();
}


#endif