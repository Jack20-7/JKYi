#include"JKYi/ProcessInfo.h"
#include"JKYi/reactor/CurrentThread.h"
#include"JKYi/FileUtil.h"

#include<algorithm>
#include<assert.h>
#include<dirent.h>
#include<pwd.h>
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/resource.h>
#include<sys/times.h>

namespace JKYi{
namespace detail{

__thread int t_numOpenedFiles = 0;

int fdDirFilter(const struct dirent* d){
    if(::isdigit(d->d_name[0])){
        ++t_numOpenedFiles;
    }
    return 0;
}

__thread std::vector<pid_t>* t_pids = NULL;

int taskDirFilter(const struct dirent* d){
    if(::isdigit(d->d_name[0])){
        t_pids->push_back(atoi(d->d_name));
    }
    return 0;
}

int scanDir(const char* dirpath,int (* filter) (const struct dirent*)){
    struct dirent** namelist = NULL;
    //scandir函数的作用就是对dirpath目录下的文件进行扫描
    //对于目录下的每一个dirent，都将他作为参数调用filter
    //如果返回0就不将该dirent加入到namelist链表中去
    //最后再通过alphasort根据文件名来排序
    int result = ::scandir(dirpath,&namelist,filter,alphasort);
    assert(namelist == NULL);
    return result;
}

JKYi::net::Timestamp g_startTime = JKYi::net::Timestamp::now();
int g_clockTicks = static_cast<int>(::sysconf(_SC_CLK_TCK));   //每秒滴答数
int g_pageSize = static_cast<int>(::sysconf(_SC_PAGE_SIZE));


}//namespace detial
}//namespace JKYi

using namespace JKYi;
using namespace JKYi::detail;
using namespace JKYi::net;

pid_t ProcessInfo::pid(){
    return ::getpid();
}

std::string ProcessInfo::pidString(){
    char buf[32];
    snprintf(buf,sizeof buf,"%d",pid());
    return buf;
}

uid_t ProcessInfo::uid(){
    return ::getuid();
}

std::string ProcessInfo::username(){
    struct passwd pwd;
    struct passwd* result = NULL;
    char buf[8192];
    const char* name = "unknownuser";

    //根据传入的用户id 去获取用户在/etc/passwd中的信息，然后将它们存储在传入的passwd结构体
    getpwuid_r(uid(),&pwd,buf,sizeof buf,&result);
    if(result){
        name = pwd.pw_name;
    }
    return name;
}

uid_t ProcessInfo::euid(){
    return ::geteuid();
}

Timestamp ProcessInfo::startTime(){
    return g_startTime;
}

int ProcessInfo::clockTicksPerSecond(){
    return g_clockTicks;
}

int ProcessInfo::pageSize(){
    return g_pageSize;
}

bool ProcessInfo::isDebugBuild(){
#ifdef NDEBUF
    return false;
#else
    return true;
#endif
}

std::string ProcessInfo::hostname(){
    char buf[256];
    if(::gethostname(buf,sizeof buf) == 0){
        buf[sizeof(buf) - 1] = '\0';
        return buf;
    }else{
        return "unknownhostname";
    }
}

std::string ProcessInfo::procname(){
    return procname(procStat()).as_string();
}

StringPiece ProcessInfo::procname(const std::string& stat){
    StringPiece name;
    size_t lp = stat.find('(');
    size_t rp = stat.find(')');
    if(lp != std::string::npos && rp != std::string::npos && lp < rp){
        name.set(stat.data() + lp + 1,static_cast<int>(rp - lp - 1));
    }
    return name;
}

std::string ProcessInfo::procStatus(){
    std::string result;
    FileUtil::readFile("/proc/self/status",65535,&result);
    return result;
}

std::string ProcessInfo::procStat(){
    std::string result;
    FileUtil::readFile("/proc/self/stat",65535,&result);
    return result;
}

std::string ProcessInfo::threadStat(){
    char buf[64];
    snprintf(buf,sizeof buf,"/proc/self/task/%d/stat",CurrentThread::tid());
    std::string result;
    FileUtil::readFile(buf,65535,&result);
    return result;
}

std::string ProcessInfo::exePath(){
    std::string result;
    char buf[1024];
    ssize_t n = ::readlink("/proc/self/exe",buf,sizeof buf);
    if(n > 0){
        result.assign(buf,n);
    }
    return result;
}

int ProcessInfo::openedFiles(){
    t_numOpenedFiles = 0;
    scanDir("/proc/self/fd",fdDirFilter);
    return t_numOpenedFiles;
}

int ProcessInfo::maxOpenFiles(){
    struct rlimit rl;
    if(::getrlimit(RLIMIT_NOFILE,&rl)){
        return openedFiles();
    }else{
        return static_cast<int>(rl.rlim_cur);
    }
}

ProcessInfo::CpuTime ProcessInfo::cpuTime(){
    ProcessInfo::CpuTime t;
    struct tms tms;
    if(::times(&tms) >= 0){
        const double hz = static_cast<double>(clockTicksPerSecond()); //每秒滴答数
        t.userSeconds = static_cast<double>(tms.tms_utime) / hz;
        t.systemSeconds = static_cast<double>(tms.tms_stime) / hz;
    }
    return t;
}

int ProcessInfo::numThreads(){
    int result = 0;
    std::string status = procStatus();
    size_t pos = status.find("Threads:");
    if(pos != std::string::npos){
        result = ::atoi(status.c_str() + pos + 8);
    }
    return result;
}

std::vector<pid_t> ProcessInfo::threads(){
   std::vector<pid_t> result;
   t_pids = &result;
   scanDir("/proc/self/task",taskDirFilter);
   t_pids = NULL;
   std::sort(result.begin(),result.end());
   return result;
}
