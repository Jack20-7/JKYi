#include"json_util.h"
#include"JKYi/util.h"

namespace JKYi{

std::string JsonUtil::GetString(const Json::Value& json,const std::string& name,
                                 const std::string& default_value){
    if(!json.isMember(name)){
        return default_value;
    }
    auto& v = json[name];
    if(v.isString()){
        return v.asString();
    }
    return default_value;
}

double JsonUtil::GetDouble(const Json::Value& json,const std::string& name,
                             double default_value){
    if(!json.isMember(name)){
       return default_value; 
    }
    auto& v = json[name];
    if(v.isDouble()){
        return v.asDouble();
    }else if(v.isString()){
        return TypeUtil::Atof(v.asString());
    }
    return default_value;
}
int32_t JsonUtil::GetInt32(const Json::Value& json,const std::string& name,
                             int32_t default_value){
    if(!json.isMember(name)){
        return default_value;
    }
    auto& v = json[name];
    if(v.isInt()){
        return v.asInt();
    }else if(v.isString()){
        return TypeUtil::Atof(v.asString());
    }
    return default_value;
}
uint32_t JsonUtil::GetUint32(const Json::Value& json,const std::string& name,
                              uint32_t default_value){
    if(!json.isMember(name)){
        return default_value;
    } 
    auto& v = json[name];
    if(v.isUInt()){
        return v.asUInt();
    }else if(v.isString()){
        return TypeUtil::Atoi(v.asString());
    }
    return default_value;
}
int64_t JsonUtil::GetInt64(const Json::Value& json,const std::string& name,
                             int64_t default_value){
    if(!json.isMember(name)){
        return default_value;
    }
    auto& v = json[name];
    if(v.isInt64()){
        return v.asInt64();
    }else if(v.isString()){
        return TypeUtil::Atoi(v.asString());
    }
    return default_value;
}
uint64_t JsonUtil::GetUint64(const Json::Value& json,const std::string& name,
                              uint64_t default_value){
    if(!json.isMember(name)){
        return default_value;
    }
    auto& v = json[name];
    if(v.isUInt64()){
        return v.asUInt64();
    }else if(v.isString()){
        return TypeUtil::Atoi(v.asString());
    }
    return default_value;
}

bool JsonUtil::FromString(Json::Value& json,const std::string& str){
    Json::Reader reader;
    return reader.parse(str,json);
}
std::string JsonUtil::ToString(const Json::Value& json){
    Json::FastWriter w;
    return w.write(json);
}

}
