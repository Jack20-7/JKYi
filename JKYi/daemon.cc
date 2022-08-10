#include"daemon.h"
#include"JKYi/log.h"
#include"JKYi/config.h"

#include<time.h>
#include<string.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<unistd.h>

namespace JKYi{

static Logger::ptr g_logger = JKYI_LOG_NAME("system");

static ConfigVar<uint32_t>::ptr g_daemon_restart_interval = 
                   JKYi::Config::Lookup("daemon.restart_interval",(uint32_t)5,
                            "daemon restart interval");

std::string ProcessInfo::toString()const{
    std::stringstream ss;
    ss << "[ProcessInfo parent_id = "<<parent_id
       << " main_id = "<<main_id
       << " parent_start_time = "<<Time2Str(parent_start_time)
       << " main_start_time = "<<Time2Str(main_start_time)
       << " restart_count = "<<restart_count <<"]";
    return ss.str();
}

static int real_start(int argc,char ** argv,
                      std::function<int(int argc,char** argv)>main_cb){
    ProcessInfoMgr::GetInstance()->main_id = getpid();
    ProcessInfoMgr::GetInstance()->main_start_time = time(0);
    return main_cb(argc,argv);
}

static int real_daemon(int argc,char** argv,
                        std::function<int (int argc,char ** argv)>main_cb){
    //第一个参数表示是否将工作目录指向根目录
    //第二个参数表示是否要关闭标准输入输出
    daemon(1,0); 

    ProcessInfoMgr::GetInstance()->parent_id = getpid();
    ProcessInfoMgr::GetInstance()->parent_start_time = time(0);
    while(true){
        pid_t pid = fork();
        if(pid == 0){
            //子进程
            ProcessInfoMgr::GetInstance()->main_id = getpid();
            ProcessInfoMgr::GetInstance()->main_start_time = time(0);
            JKYI_LOG_INFO(g_logger)<<"sub process start,pid ="<<getpid(); 
            return real_start(argc,argv,main_cb);
        }else if(pid < 0){
            //创建子进程失败
            JKYI_LOG_ERROR(g_logger)<<"fork fail,return = "<<pid
                                    <<" errno = "<<errno
                                    <<" errstr = "<<strerror(errno);
            return -1;
        }else{
            //父进程
            int status = 0;
            waitpid(pid,&status,0);
            if(status){
              //非正常退出
              if(status == 9){
                  JKYI_LOG_INFO(g_logger)<<"killed";
                  break;
              }else{
                  JKYI_LOG_ERROR(g_logger)<<"child crash,pid = "<<pid
                                          <<" status = "<<status;
              }
            }else{
                //子进程执行完毕，正常退出
                JKYI_LOG_INFO(g_logger)<<"child finished,pid = "<<pid;
                break;
            }
            ProcessInfoMgr::GetInstance()->restart_count += 1;
            sleep(g_daemon_restart_interval->getValue());
        }
    }
    return 0;
}

int start_daemon(int argc,char ** argv,
                 std::function<int(int argc,char ** argv)>main_cb,
                  bool is_daemon){

    if(!is_daemon){
       ProcessInfoMgr::GetInstance()->parent_id = getpid();
       ProcessInfoMgr::GetInstance()->parent_start_time = time(0);
       return real_start(argc,argv,main_cb);
    }
    return real_daemon(argc,argv,main_cb);
}
         
}
