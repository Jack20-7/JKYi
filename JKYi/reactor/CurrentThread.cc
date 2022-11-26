#include"JKYi/reactor/CurrentThread.h"
#include"JKYi/timestamp.h"

#include<cxxabi.h>
#include<execinfo.h>
#include<stdlib.h>
#include<time.h>

namespace JKYi{
namespace CurrentThread{

__thread int t_cachedTid = 0;
__thread char t_tidString[32];
__thread int t_tidStringLength = 6;
__thread const char * t_threadName = "unknow";

void sleepUsec(int64_t usec){
    struct timespec ts = {0,0};
    ts.tv_sec = static_cast<time_t>(usec / (1000 * 1000));
    ts.tv_nsec = static_cast<long>(usec % (1000 * 1000) * 1000);
    ::nanosleep(&ts,NULL);
}

}
}
