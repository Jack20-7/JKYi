#include"JKYi/reactor/examples/procmon/plot.h"
#include"JKYi/reactor/Logging.h"
#include"JKYi/reactor/EventLoop.h"
#include"JKYi/reactor/http/HttpRequest.h"
#include"JKYi/reactor/http/HttpResponse.h"
#include"JKYi/reactor/http/HttpServer.h"
#include"JKYi/FileUtil.h"
#include"JKYi/ProcessInfo.h"
#include"JKYi/reactor/Buffer.h"
#include"JKYi/log.h"

#include"/usr/local/include/boost/algorithm/string/replace.hpp"
#include"/usr/local/include/boost/circular_buffer.hpp"

#include<sstream>
#include<type_traits>
#include<dirent.h>
#include<stdarg.h>
#include<stdio.h>
#include<sys/stat.h>
#define __STDC_FORMAT_MACROS
#include<inttypes.h>

using namespace JKYi;
using namespace JKYi::net;

static JKYi::Logger::ptr g_logger = JKYI_LOG_NAME("system");

//用来表示进程的状态信息
struct StatData{
public:
    void parse(const char* startAtState,int kbPerPage){
        std::stringstream iss(startAtState);

        iss >> state;
        iss >> ppid >> pgrp >> session >> tty_nr >> tpgid >> flags;
        iss >> minflt >> cminflt >> majflt >> cmajflt;
        iss >> utime >> stime >> cutime >> cstime;
        iss >> priority >> nice >> num_threads >> itrealvalue >> starttime;

        long vsize,rss;
        iss >> vsize >> rss >> rsslim;
        vsizeKb = vsize / 1024;
        rssKb = rss * kbPerPage;
    }

    char  state;      //进程的状态
    int   ppid;       //父进程的pid
    int   pgrp;       //所在线程组id
    int   session;    //所在session的id
    int   tty_nr;     //进程的tty终端的设备号
    int   tpgid;
    int   flags;

    long  minflt;     
    long  cminflt;
    long  majflt;
    long  cmajflt;

    long  utime;
    long  stime;
    long  cutime;    //所有子进程在用户空间执行的时间
    long  cstime;    //所有子进程在内核空间执行的时间

    long  priority;
    long  nice;        //静态优先级
    long  num_threads;
    long  itrealvalue;
    long  starttime;   //进程创建的时间

    long  vsizeKb;     //虚拟内存使用情况
    long  rssKb;       //物理内存使用情况
    long  rsslim;      //物理内存的软限制
};

static_assert(std::is_pod<StatData>::value,"StateData should be POD");

class Procmon : Noncopyable{
public:
    Procmon(EventLoop* loop,pid_t pid,uint16_t port,const char* procname)
        :kClockTicksPerSecond_(JKYi::ProcessInfo::clockTicksPerSecond()),
         kbPerPage_(JKYi::ProcessInfo::pageSize() / 1024),
         kBootTime_(getBootTime()),
         pid_(pid),
         server_(loop,JKYi::Address::LookupAnyIPAddress("0.0.0.0",std::to_string(port)),getName()),
         procname_(procname ? procname : JKYi::ProcessInfo::procname(readProcFile("stat")).as_string()),
         hostname_(JKYi::ProcessInfo::hostname()),
         cmdline_(getCmdLine()),
         ticks_(0),
         cpu_usage_(600 / kPeriod_), //10分钟   
         cpu_chart_(640,100,600,kPeriod_){
         {
             std::string cwd = readLink("cwd");
             if(::chdir(cwd.c_str())){
                 LOG_ERROR << "Cannot chdir() to " << cwd;
             }
         }

         char cwd[1024];
         if(::getcwd(cwd,sizeof cwd)){
             LOG_INFO << "Current cwd = " << cwd;
         }

         memset(&lastStatData_,0,sizeof(lastStatData_));

         server_.setHttpCallback(
                 std::bind(&Procmon::onRequest,this,_1,_2));
     }

     void start(){
         tick();
         server_.getLoop()->runEvery(kPeriod_,std::bind(&Procmon::tick,this));
         server_.start();
     }
private:

    std::string getName()const{
        char name[256];
        snprintf(name,sizeof name,"procmon - %d",pid_);
        return name;
    }

    void onRequest(const HttpRequest& req,HttpResponse* rsp){
        rsp->setStatusCode(HttpResponse::k200Ok);
        rsp->setStatusMessage("OK");
        rsp->setContentType("text/plain");
        rsp->addHeader("Server","JKYi-Procmon");

        if(req.path() == "/"){
            rsp->setContentType("text/html");
            fillOverview(req.query());
            rsp->setBody(response_.retrieveAllAsString());
        }else if(req.path() == "/cmdline"){
            rsp->setBody(cmdline_);
        }else if(req.path() == "/cpu.png"){
            std::vector<double> cpu_usage;
            for(size_t i = 0;i < cpu_usage_.size();++i){
                cpu_usage.push_back(cpu_usage_[i].cpuUsage(kPeriod_,kClockTicksPerSecond_));
            }
            std::string png = cpu_chart_.plotCpu(cpu_usage);
            rsp->setContentType("image/png");
            rsp->setBody(png);
        }else if(req.path() == "/environ"){
            rsp->setBody(getEnviron());
        }else if(req.path() == "/io"){
            rsp->setBody(readProcFile("io"));
        }else if(req.path() == "/limits"){
            rsp->setBody(readProcFile("limits"));
        }else if(req.path() == "/maps"){
            rsp->setBody(readProcFile("maps"));
        }else if(req.path() == "/smaps"){
            rsp->setBody(readProcFile("smaps"));
        }else if(req.path() == "/status"){
            rsp->setBody(readProcFile("status"));
        }else if(req.path() == "/files"){
            listFiles();
            rsp->setBody(response_.retrieveAllAsString());
        }else if(req.path() == "threadds"){
            listThreads();
            rsp->setBody(response_.retrieveAllAsString());
        }else{
            rsp->setStatusCode(HttpResponse::k404NotFound);
            rsp->setStatusMessage("Not Found");
            rsp->setCloseConnection(true);
        }
    }

    void fillOverview(const std::string& query){
        response_.retrieveAll();
        Timestamp now = Timestamp::now();
        appendResponse("<html><head><title>%s on %s</title>\n",
                   procname_.c_str(), hostname_.c_str());

        fillRefresh(query);

        appendResponse("</head><body>\n");

        std::string stat = readProcFile("stat");
        if(stat.empty()){
            appendResponse("<h1>PID %d doesn't exist.</h1></body></html>", pid_);
            return ;
        }
        int pid = std::atoi(stat.c_str());
        assert(pid == pid_);

        StringPiece procname = ProcessInfo::procname(stat);
        appendResponse("<h1>%s on %s</h1>\n",
                                   procname.as_string().c_str(), hostname_.c_str());
        response_.append("<p>Refresh <a href=\"?refresh=1\">1s</a> ");
        response_.append("<a href=\"?refresh=2\">2s</a> ");
        response_.append("<a href=\"?refresh=5\">5s</a> ");
        response_.append("<a href=\"?refresh=15\">15s</a> ");
        response_.append("<a href=\"?refresh=60\">60s</a>\n");
        response_.append("<p><a href=\"/cmdline\">Command line</a>\n");
        response_.append("<a href=\"/environ\">Environment variables</a>\n");
        response_.append("<a href=\"/threads\">Threads</a>\n");

        appendResponse("<p>Page generated at %s (UTC)", now.toFormattedString().c_str());

        response_.append("<p><table>");
        StatData statData;
        memset(&statData,0,sizeof(statData));
        statData.parse(procname.end() + 1,kbPerPage_);

        appendTableRow("PID", pid);
        Timestamp started(getStartTime(statData.starttime));
        appendTableRow("Started at", started.toFormattedString(false /*showMicroseconds*/) + " (UTC)");
        appendTableRowFloat("Uptime (s)", timeDifference(now, started));
        appendTableRow("Executable", readLink("exe"));
        appendTableRow("Current dir", readLink("cwd"));
        appendTableRow("State", getState(statData.state));
        appendTableRowFloat("User time (s)", getSeconds(statData.utime));
        appendTableRowFloat("System time (s)", getSeconds(statData.stime));
        appendTableRow("VmSize (KiB)", statData.vsizeKb);
        appendTableRow("VmRSS (KiB)", statData.rssKb);
        appendTableRow("Threads", statData.num_threads);
        appendTableRow("CPU usage", "<img src=\"/cpu.png\" height=\"100\" witdh=\"640\">");

        appendTableRow("Priority", statData.priority);
        appendTableRow("Nice", statData.nice);
        appendTableRow("Minor page faults", statData.minflt);
        appendTableRow("Major page faults", statData.majflt);

        response_.append("</table>");
        response_.append("</body></html>");
    }

    //更新 刷新时间
    void fillRefresh(const std::string& query){
        size_t p = query.find("refresh=");
        if(p != std::string::npos){
            int seconds = atoi(query.c_str() + p + 8);
            if(seconds > 0){
                appendResponse("<meta http-equiv=\"refresh\" content=\"%d\">\n", seconds);
            }
        }
    }

    static int dirFilter(const struct dirent* d){
        return (d->d_name[0] != '.');
    }

    static char getDirType(char d_type){
        switch(d_type){
            case DT_REG: 
                return '-';
            case DT_DIR:
                return 'd';
            case DT_LNK:
                return 'l';
            default:
                return '?';
        }
    }

    void listFiles(){
        struct dirent** namelist = NULL;
        int result = ::scandir(".",&namelist,dirFilter,alphasort);
        for(int i = 0 ;i < result;++i){
            struct stat stat;
            if(::lstat(namelist[i]->d_name,&stat) == 0){
                //该目录最后一个被修改的时间
                Timestamp mtime(stat.st_mtime * Timestamp::kMicroSecondsPerSecond);
                int64_t size = stat.st_size;
                appendResponse("%c %9" PRId64 " %s %s", getDirType(namelist[i]->d_type), size,
                       mtime.toFormattedString(/*showMicroseconds=*/false).c_str(),
                       namelist[i]->d_name);
                if(namelist[i]->d_type == DT_LNK){
                    char link[1024];
                    ssize_t len = ::readlink(namelist[i]->d_name,link,sizeof link - 1);
                    if(len > 0){
                        link[len] = '\0';
                        appendResponse(" -> %s",link);
                    }
                }
                appendResponse("\n");
            }
            ::free(namelist[i]);
        }
        ::free(namelist);
    }
    void listThreads(){
        response_.retrieveAll();
        //TODO  implement it
    }

    std::string readProcFile(const char* basename){
        char filename[256];
        snprintf(filename,sizeof filename,"/proc/%d/%s",pid_,basename);

        std::string content;
        FileUtil::readFile(filename,1024 * 1024,&content);

        return content;
    }

    std::string readLink(const char* basename){
        char filename[256];
        snprintf(filename,sizeof filename,"/proc/%d/%s",pid_,basename);

        char link[1024];
        ssize_t len = ::readlink(filename,link,sizeof link);

        std::string result;
        if(len > 0){
            result.assign(link,len);
        }
        return result;
    }

    int appendResponse(const char* fmt, ...) __attribute__ ((format (printf, 2, 3)));
    void appendTableRow(const char* name, long value){
        appendResponse("<tr><td>%s</td><td>%ld</td></tr>\n", name, value);
    }
    void appendTableRowFloat(const char* name, double value){
         appendResponse("<tr><td>%s</td><td>%.2f</td></tr>\n", name, value);
    }
    void appendTableRow(const char* name, StringArg value){
         appendResponse("<tr><td>%s</td><td>%s</td></tr>\n", name, value.c_str());
    } 

    std::string getCmdLine(){
        //将cmdline返回的文本中的'0'替换为\n\t返回，并且保持原有的文本不变
        return boost::replace_all_copy(readProcFile("cmdline"),std::string(1,'\0'),"\n\t");
    }

    std::string getEnviron(){
        return boost::replace_all_copy(readProcFile("environ"),std::string(1,'\0'),"\n");
    }

    Timestamp getStartTime(long startTime){
        return Timestamp(Timestamp::kMicroSecondsPerSecond * kBootTime_ 
                             + Timestamp::kMicroSecondsPerSecond * startTime / kClockTicksPerSecond_);
    }

    double getSeconds(long ticks){
        return static_cast<double>(ticks) / kClockTicksPerSecond_;
    }

    void tick(){
        //收集CPU画图相关的参数
        std::string stat = readProcFile("stat");
        if(stat.empty()){
            return ;
        }
        StringPiece procname = ProcessInfo::procname(stat);
        StatData statData;
        memset(&statData,0,sizeof(statData));
        statData.parse(procname.end() + 1,kbPerPage_);

        if(ticks_ > 0){
            CpuTime time;
            time.userTime_ = std::max(0,static_cast<int>(statData.utime - lastStatData_.utime));
            time.sysTime_ = std::max(0,static_cast<int>(statData.stime - lastStatData_.stime));

            cpu_usage_.push_back(time);
        }
        lastStatData_ = statData;
        ++ticks_;
    }

    static const char* getState(char state){
        switch(state){
            case 'R':
                return "Running";
            case 'S':
                return "Sleeping";
            case 'D':
                return "Disk sleep";
            case 'Z':
                return "Zombie";
            default:
                return "Unknown";
        }
    }

    static long getLong(const std::string& status,const char* key){
        long result = 0;
        size_t pos = status.find(key);
        if(pos != std::string::npos){
            return ::atol(status.c_str() + pos + strlen(key));
        }
        return result;
    }

    static long getBootTime(){
        std::string stat;
        FileUtil::readFile("/proc/stat",65536,&stat);
        return getLong(stat,"btime ");
    }

    struct CpuTime{
        int userTime_;
        int sysTime_;
        //计算出cpu的使用率
        double cpuUsage(double kPeriod,double kClockTicksPerSecond)const{
            //该周期一共执行的滴答数 / 该周期一共有多少个滴答数
            return (userTime_ + sysTime_) / (kClockTicksPerSecond * kPeriod_);
        }
    };


    const static int kPeriod_ = 2.0; //刷新的间隔,也是每一次统计的周期
    const int kClockTicksPerSecond_; //cpu每秒滴答数
    const int kbPerPage_;            //物理页的大小
    const long kBootTime_;          
    const pid_t pid_;              //要监控的进程id
    HttpServer server_;
    const std::string procname_;
    const std::string hostname_;
    const std::string cmdline_;
    int ticks_;
    StatData lastStatData_;

    boost::circular_buffer<CpuTime> cpu_usage_;
    Plot cpu_chart_;

    Buffer response_;
};

int Procmon::appendResponse(const char* fmt,...){
    char buf[1024];
    va_list ap;
    va_start(ap,fmt);
    int ret = vsnprintf(buf,sizeof buf,fmt,ap);
    va_end(ap);
    response_.append(buf);

    return ret;
}

bool processExists(pid_t pid){
    char filename[256];
    snprintf(filename,sizeof filename,"/proc/%d/stat",pid);
    return ::access(filename,R_OK) == 0;
}


int main(int argc,char** argv){
    if(argc < 3){
        printf("Usage:%s pid port [name]\n",argv[0]);
        return 0;
    }

    int pid = atoi(argv[1]);
    if(!processExists(pid)){
        printf("Process %d doesn't exist\n",pid);
        return 1;
    }

    g_logger->setLevel(LogLevel::INFO);
    EventLoop loop;
    uint16_t port = static_cast<uint16_t>(atoi(argv[2]));

    Procmon procmon(&loop,pid,port,argc > 3 ? argv[3] : NULL);
    procmon.start();
    loop.loop();

    return 0;
}

