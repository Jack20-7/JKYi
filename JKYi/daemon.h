#ifndef _JKYI_DAEMON_H_
#define _JKYI_DAEMON_H_

#include<unistd.h>
#include<functional>
#include"JKYi/singleton.h"

namespace JKYi{
  
//存储进程相关信息的结构体
struct ProcessInfo{
   //父进程id
   pid_t parent_id = 0;
   //该子进程id
   pid_t main_id = 0;
   //父进程开始运行的时间
   uint64_t parent_start_time = 0;
   //子进程开始运行的时间
   uint64_t main_start_time = 0;
   //该子进程重启的次数
   uint32_t restart_count = 0;

   std::string toString()const;
};

typedef JKYi::Singleton<ProcessInfo> ProcessInfoMgr;

//argc  启动时传入的参数个数
//argv  启动时传入的参数
//main_cb 要执行的回调函数
//is_daemon  是否要以守护进程的方式启动
int start_daemon(int argc,char ** argv,std::function<int(int argc,char ** argv)>main_cb,bool is_daemon);
}

#endif
