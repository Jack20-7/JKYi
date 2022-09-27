#ifndef _JKYI_CURRENT_THREAD_H_
#define _JKYI_CURRENT_THREAD_H_

namespace JKYi{
namespace CurrentThread{

extern __thread int t_cachedTid;        //缓存当前线程的pid_t
extern __thread char t_tidString[32];   //用于打日志
extern __thread int t_tidStringLength;
extern __thread const char * t_threadName;  //缓存线程的名称

void cacheTid();

inline int tid(){
    if(__builtin_expect(t_cachedTid == 0,0)){
        //如果没有缓存有pid
        cacheTid();
    }
    return t_cachedTid;
}

inline const char * tidString(){
    return t_tidString;
}

inline int tidStringLength(){
    return t_tidStringLength;
}
inline const char * name(){
    return t_threadName;
}

bool isMainThread();

}
}

#endif
