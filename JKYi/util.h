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
#include<iomanip>
#include<sys/time.h>
#include<boost/lexical_cast.hpp>

//该文件中定义的是一些常用的函数
namespace JKYi{
//获取线程id的函数
pid_t GetThreadId();  

//返回当前协程的id
uint64_t GetFiberId();

//返回当前的栈帧的信息
//第一个参数的vector用来存储栈帧的信息,作为传出参数
//第二个参数表示的用户想要输出栈帧的层数
//第三个层数表示用户想要从那一层开始输出
void Backtrace(std::vector<std::string>&bt,int size=64,int skip=1);

//该函数可作为用户调用的接口
std::string BacktraceToString(int size=64,int skip=2,const std::string&prefix="  ");

//获得当前时间对应的毫秒数
uint64_t GetCurrentMS();

//获得当前时间对应的微妙数
uint64_t GetCurrentUS();

//该函数就是将MS时间戳转换为我们平时理解的时间
std::string Time2Str(time_t ts = time(0),const std::string& format 
                                                 = "%Y-%m-%d %H:%M:%S");

//封装一个专门对文件操作的类
//
class FSUtil{
public:
    //将path路径下后缀中带有subfix的文件的绝对路径放入到files容器中去
    static void ListAllFile(std::vector<std::string>& files,
                            const std::string& path,
                            const std::string& subfix);
    //保证dirname对于的目录存在
    static bool Mkdir(const std::string& dirname);
    //对于的进程是否执行
    static bool IsRunningPidfile(const std::string& pidfile);

};

class TypeUtil{
public:
    static int8_t ToChar(const std::string& str);
    static int64_t Atoi(const std::string& str);
    static double Atof(const std::string& str);

    static int8_t ToChar(const char * str);
    static int64_t Atoi(const char * str);
    static double Atof(const char * str);
};

template<class Map ,class K,class V>
V GetParamValue(const Map& m,const K& k,const V& def = V()){
    auto it = m.find(k);
    if(it == m.end()){
        return def;
    }
    try{
        return boost::lexical_cast<V>(it->second);
    }catch(...){
    }
    return def;
}


}


#endif
