#include"JKYi/log.h"
#include"JKYi/config.h"
#include<yaml-cpp/yaml.h>

#include<iostream>

int main(int argc,char ** argv){
    const YAML::Node& node = YAML::LoadFile("/home/admin/workSpace/bin/conf/log.yml");
    JKYi::Config::LoadFromYaml(node); 
    JKYi::Logger::ptr r_logger  = JKYI_LOG_NAME("root");
    std::cout << r_logger->toYamlString() << std::endl;

    JKYI_LOG_DEBUG(r_logger) << " this is debug";
    JKYI_LOG_INFO(r_logger) << " this is info";
    JKYI_LOG_WARN(r_logger) << " this is warn";
    JKYI_LOG_ERROR(r_logger) << " this is error";
    JKYI_LOG_FATAL(r_logger) << " this is fatal";

    return 0;
}
