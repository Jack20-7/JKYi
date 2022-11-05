#include"JKYi/reactor/Logging.h"
#include"JKYi/reactor/AsyncLogging.h"
#include"JKYi/timestamp.h"

#include"JKYi/log.h"
#include"JKYi/config.h"

#include<unistd.h>
#include<stdio.h>
#include<string>
#include<iostream>

using namespace JKYi;
using namespace JKYi::net;

long long g_total = 0;
int main(){
    YAML::Node root = YAML::LoadFile("/tmp/log.yml");
    JKYi::Config::LoadFromYaml(root);

    JKYi::Logger::ptr logger = JKYI_LOG_NAME("root");
    //记录开始时间
    Timestamp start = Timestamp::now();
    int n = 1000 * 1000;
    std::string longStr(3000,'x');
    longStr += " ";
    for(int i = 0;i < n;++i){
        LOG_INFO << longStr;
        //JKYI_LOG_INFO(logger) << longStr;
        g_total += longStr.size();
    }
    Timestamp end = Timestamp::now();
    double seconds = timeDifference(end,start);
    printf("%f seocnds,%lld bytes,%10.2f msg/s,%.2f MiB/s\n",
              seconds,g_total,n / seconds,g_total / seconds / (1024 * 1024));
    return 0;
}
