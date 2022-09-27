#include"JKYi/timestamp.h"
#include"JKYi/macro.h"

#include<sys/time.h>
#include<stdio.h>
#include<iostream>
#include<sstream>

namespace JKYi{
namespace net{

std::string Timestamp::toString()const {
  std::stringstream ss;
  int64_t seconds = m_microSecondsSinceEpoch / kMicroSecondsPerSecond;
  int64_t microseconds = m_microSecondsSinceEpoch % kMicroSecondsPerSecond;
  ss << seconds << "." << microseconds;
  return ss.str();
}
std::string Timestamp::toFormattedString(bool showMicroseconds)const{
   char buf[64] = {0};
   time_t seconds = static_cast<time_t>(m_microSecondsSinceEpoch /
                                                   kMicroSecondsPerSecond);
   struct tm tm_time;
   gmtime_r(&seconds,&tm_time);
   if(showMicroseconds){
       int microseconds = static_cast<int>(m_microSecondsSinceEpoch % 
                                               kMicroSecondsPerSecond);
       snprintf(buf, sizeof(buf), "%4d%02d%02d %02d:%02d:%02d.%06d",
             tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
             tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec,
             microseconds);
   }else{
       snprintf(buf, sizeof(buf), "%4d%02d%02d %02d:%02d:%02d",
             tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
             tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec
             );
   }
   return buf;
}

Timestamp Timestamp::now(){
    //精度是微妙
    //gettimeofday在x86的机器上不是系统调用，是在用户态实现的，所以开销小，速度快
    struct timeval tv;
    gettimeofday(&tv,NULL);
    int64_t seconds = tv.tv_sec;
    return Timestamp(seconds * kMicroSecondsPerSecond + tv.tv_usec);
}
}
}
