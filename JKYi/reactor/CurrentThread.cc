#include"JKYi/reactor/CurrentThread.h"

#include<cxxabi.h>
#include<execinfo.h>
#include<stdlib.h>

namespace JKYi{
namespace CurrentThread{

__thread int t_cachedTid = 0;
__thread char t_tidString[32];
__thread int t_tidStringLength = 6;
__thread const char * t_threadName = "unknow";

}
}
