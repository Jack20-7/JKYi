#ifndef _JKYI_PROCESSINFO_H_
#define _JKYI_PROCESSINFO_H_

#include"JKYi/reactor/StringPiece.h"
#include"JKYi/Types.h"
#include"JKYi/timestamp.h"

#include<vector>
#include<sys/types.h>
#include<string>

namespace JKYi{
namespace ProcessInfo{

pid_t pid();

std::string pidString();

uid_t uid();

std::string username();

uid_t euid();

net::Timestamp startTime();

int clockTicksPerSecond();

int pageSize();

bool isDebugBuild();

std::string hostname();

std::string procname();

JKYi::net::StringPiece procname(const std::string& stat);

// /proc/self/status
std::string procStatus();

// /proc/self/stat
std::string procStat();

// /proc/self/task/tid/stat
std::string threadStat();

// /proc/self/exe
std::string exePath();

int openedFiles();

int maxOpenFiles();

struct CpuTime{
    double userSeconds;
    double systemSeconds;

    CpuTime()
        :userSeconds(0.0),
         systemSeconds(0.0){}
    double total()const {
        return userSeconds + systemSeconds;
    }
};

CpuTime cpuTime();

int numThreads();

std::vector<pid_t> threads();
}//namespace ProcessInfo
}//namespace JKYi

#endif
