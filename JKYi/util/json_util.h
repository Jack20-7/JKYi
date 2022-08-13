#ifndef _JKYI_JSON_UTIL_H_
#define _JKYI_JSON_UTIL_H_
#include<string>
#include<iostream>
#include<jsoncpp/json/json.h>

namespace JKYi{

class JsonUtil{
public:
    static std::string GetString(const Json::Value& json,const std::string& name,const std::string& default_value = "");
    static double GetDouble(const Json::Value& json,const std::string& name,double default_value = 0);
    static int32_t GetInt32(const Json::Value& json,const std::string& name,int32_t default_value = 0);
    static uint32_t GetUint32(const Json::Value& json,const std::string& name,uint32_t default_value = 0);
    static int64_t GetInt64(const Json::Value& json,const std::string& name,int64_t default_value = 0);
    static uint64_t GetUint64(const Json::Value& json,const std::string& name,uint64_t default_value = 0);

    static bool FromString(Json::Value& json,const std::string& str);
    static std::string ToString(const Json::Value& json);
};
}

#endif
