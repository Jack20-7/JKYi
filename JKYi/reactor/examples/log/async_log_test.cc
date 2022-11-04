#include"JKYi/reactor/Logging.h"
#include"JKYi/reactor/AsyncLogging.h"

#include<unistd.h>

using namespace JKYi;
using namespace JKYi::net;
int main(){
      LOG_TRACE << "hello,trace";
      LOG_DEBUG << "hello,debug";
      LOG_INFO << "hello,info";
      LOG_WARN << "hello,warn";
      LOG_ERROR << "hello,error";
    return 0;
}
