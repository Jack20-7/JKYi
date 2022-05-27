#include"util.h"

namespace JKYi{
   //用来返回当前线程的id
   pid_t GetThreadId(){
       return syscall(SYS_gettid);
   }
   //返回当前协程的id
   u_int32_t GetFiberId(){
       return 0;
   }
}