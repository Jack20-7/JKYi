#ifndef _JKYI_UTIL_H_
#define _JKYI_UTIL_H_

#include<pthread.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/syscall.h>
#include<stdio.h>
#include<stdint.h>
#include<vector>
#include<string>
//该文件中定义的是一些常用的函数
namespace JKYi{
    //获取线程id的函数
   pid_t GetThreadId();  
   //返回当前协程的id
   u_int32_t GetFiberId();
   //返回当前的栈帧的信息
   //第一个参数的vector用来存储栈帧的信息,作为传出参数
   //第二个参数表示的用户想要输出栈帧的层数
   //第三个层数表示用户想要从那一层开始输出
   void Backtrace(std::vector<std::string>&bt,int size=64,int skip=1);
   //该函数可作为用户调用的接口
   std::string BacktraceToString(int size=64,int skip=2,const std::string&prefix="  ");

}


#endif
