#include"util.h"
#include"log.h"
#include"fiber.h"

#include<execinfo.h>
#include<string>
#include<unistd.h>
#include<dirent.h>
#include<string.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<ifaddrs.h>
#include<arpa/inet.h>
#include<stdlib.h>


namespace JKYi{

static JKYi::Logger::ptr g_logger=JKYI_LOG_NAME("system");
   //用来返回当前线程的id
pid_t GetThreadId(){
       return syscall(SYS_gettid);
}
   //返回当前协程的id
uint64_t GetFiberId(){
      return Fiber::GetFiberId();
}
   
void Backtrace(std::vector<std::string>&bt,int size,int skip){
	   void ** array=(void**)malloc(sizeof(void*)*size);
	   size_t s=::backtrace(array,size);
	   
	   char**strings=backtrace_symbols(array,s);
	   if(strings==nullptr){
          JKYI_LOG_ERROR(g_logger)<<"backtrace_symbols error";
		  return ;
	   }
	   for(size_t i=skip;i<s;++i){
		   bt.push_back(strings[i]);
	   }
	   free(array);
	   free(strings);
   }
std::string BacktraceToString(int size,int skip,const std::string&prefix){
      std::vector<std::string>bt;
	  Backtrace(bt,size,skip);
	  std::stringstream ss;
	  for(size_t i=0;i<bt.size();++i){
       ss<<prefix<<bt[i]<<std::endl;
	  }
	  return ss.str();
}
//获得当前时间对应的毫秒数
uint64_t GetCurrentMS(){
    struct timeval tv;
	gettimeofday(&tv,NULL);
	return tv.tv_sec*1000ul+tv.tv_usec/1000;
}
//获得当前的微秒数
uint64_t GetCurrentUS(){
	struct timeval tv;
	gettimeofday(&tv,NULL);
	return tv.tv_sec*1000*1000ul+tv.tv_usec;
}

std::string Time2Str(time_t ts,const std::string& format){
    struct tm tm;
    localtime_r(&ts,&tm);
    char buf[64];
    strftime(buf,sizeof(buf),format.c_str(),&tm);
    return buf; 
}

void FSUtil::ListAllFile(std::vector<std::string>& files,
                         const std::string& path,
                         const std::string& subfix){
    //判断传入的路径是否存在
    if(access(path.c_str(),0) != 0){
        return ;
    }

    DIR * dir = opendir(path.c_str());
    if(dir == nullptr){
        return ;
    }
    struct dirent * dp =nullptr;
    while((dp = readdir(dir)) != nullptr){
        //如果是目录的话，就需要进行递归
        if(dp->d_type == DT_DIR){
            if(!strcmp(dp->d_name,".") 
                || !strcmp(dp->d_name,"..")){
                continue;
            }
            ListAllFile(files,path + "/" + dp->d_name,subfix);
        }else if(dp->d_type == DT_REG){
            //如果是普通的文件
            std::string filename(dp->d_name);
            if(subfix.empty()){
                files.push_back(path + "/" + filename);
            }else{
                if(filename.size() < subfix.size()){
                    continue;
                }
                if(filename.substr(filename.length() - subfix.length()) == subfix){
                  files.push_back(path + "/" + filename);
                }
            }
        }
    }
    closedir(dir);
}

static int __lstat(const char* file,struct stat* st = nullptr){
    struct stat lst;
    int ret = lstat(file,&lst);
    if(st){
        *st = lst;
    }
    return ret;
}
static int __mkdir(const char * dirname){
    //判断目录是否存在
    //如果存在的话，返回0
    //不存在返回-1
    if(access(dirname,F_OK) == 0){
        return 0;
    }
    return mkdir(dirname,S_IRWXU | S_IROTH | S_IXOTH);
}

bool FSUtil::Mkdir(const std::string& dirname){
   //如果已经存在了
   if(__lstat(dirname.c_str()) == 0){
       return true;
   }
   char * path = strdup(dirname.c_str());
   char * ptr = strchr(path + 1,'/');
   do{
       //对目录继续创建
       for(;ptr;*ptr = '/',ptr = strchr(ptr+1,'/')){
           *ptr = '\0';
           if(__mkdir(path) != 0){
               break;
           }
       }
       if(ptr != nullptr){
           break;
       }else if(__mkdir(path) != 0){
           break;
       }
       //执行到这里就表示目录创建完毕
       free(path);
       return true;
   }while(0);
   free(path);
   return false; 
}
bool FSUtil::IsRunningPidfile(const std::string& pidfile){
     if(__lstat(pidfile.c_str()) != 0){
         return false;
     }
     std::ifstream ifs(pidfile);
     std::string line;
     if(!ifs || !std::getline(ifs,line)){
         return false;
     }
     if(line.empty()){
       return false;
     }
     pid_t pid = atoi(line.c_str());
     if(pid <= 1){
         return false;
     }
     if(kill(pid,0) != 0){
         return false;
     }
     return true;
}

int8_t TypeUtil::ToChar(const std::string& str){
    if(str.empty()){
        return 0;
    }
    return *str.begin();
}
int64_t TypeUtil::Atoi(const std::string& str){
    if(str.empty()){
        return 0;
    }
   return strtoull(str.c_str(),nullptr,10);
}
double TypeUtil::Atof(const std::string& str){
    if(str.empty()){
        return 0;
    }
   return atof(str.c_str());
}

int8_t TypeUtil::ToChar(const char * str){
    if(str == nullptr){
        return 0;
    }
    return str[0];
}
int64_t TypeUtil::Atoi(const char * str){
    if(str == nullptr){
        return 0;
    }
    return strtoull(str,nullptr,10);
}
double TypeUtil::Atof(const char * str){
    if(str == nullptr){
        return 0;
    }
    return atof(str);
}
   
}

