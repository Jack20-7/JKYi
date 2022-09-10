#include"dynamic_table.h"
#include<sstream>

namespace JKYi{
namespace http2{

#define STATIC_HEADERS(XX)\
    XX("", "") \
    XX(":authority", "") \
    XX(":method", "GET") \
    XX(":method", "POST") \
    XX(":path", "/") \
    XX(":path", "/index.html") \
    XX(":scheme", "http") \
    XX(":scheme", "https") \
    XX(":status", "200") \
    XX(":status", "204") \
    XX(":status", "206") \
    XX(":status", "304") \
    XX(":status", "400") \
    XX(":status", "404") \
    XX(":status", "500") \
    XX("accept-charset", "") \
    XX("accept-encoding", "gzip, deflate") \
    XX("accept-language", "") \
    XX("accept-ranges", "") \
    XX("accept", "") \
    XX("access-control-allow-origin", "") \
    XX("age", "") \
    XX("allow", "") \
    XX("authorization", "") \
    XX("cache-control", "") \
    XX("content-disposition", "") \
    XX("content-encoding", "") \
    XX("content-language", "") \
    XX("content-length", "") \
    XX("content-location", "") \
    XX("content-range", "") \
    XX("content-type", "") \
    XX("cookie", "") \
    XX("date", "") \
    XX("etag", "") \
    XX("expect", "") \
    XX("expires", "") \
    XX("from", "") \
    XX("host", "") \
    XX("if-match", "") \
    XX("if-modified-since", "") \
    XX("if-none-match", "") \
    XX("if-range", "") \
    XX("if-unmodified-since", "") \
    XX("last-modified", "") \
    XX("link", "") \
    XX("location", "") \
    XX("max-forwards", "") \
    XX("proxy-authenticate", "") \
    XX("proxy-authorization", "") \
    XX("range", "") \
    XX("referer", "") \
    XX("refresh", "") \
    XX("retry-after", "") \
    XX("server", "") \
    XX("set-cookie", "") \
    XX("strict-transport-security", "") \
    XX("transfer-encoding", "") \
    XX("user-agent", "") \
    XX("vary", "") \
    XX("via", "") \
    XX("www-authenticate", "")

//静态表初始化
static std::vector<std::pair<std::string,std::string>> s_static_headers = {
#define XX(k,v) {k,v},
    STATIC_HEADERS(XX)
#undef XX
};

std::pair<std::string,std::string> DynamicTable::GetStaticHeaders(uint32_t idx){
    return s_static_headers[idx];
}
int32_t DynamicTable::GetStaticHeadersIdx(const std::string& name){
    for(int i = 1;i < (int)s_static_headers.size();++i){
        if(s_static_headers[i].first == name){
            return i;
        }
    }
    return -1;
}
std::pair<int32_t,bool> DynamicTable::GetStaticHeadersPair(const std::string& name, 
                                                  const std::string& val){
    std::pair<int32_t,bool> rt = std::make_pair(-1,false);
    for(int i = 0;i < (int)s_static_headers.size();++i){
       if(s_static_headers[i].first == name){
           if(rt.first == -1){
               rt.first = i;
           }
       }else{
           continue;
       }
       if(s_static_headers[i].second == val){
           rt.first = i;
           rt.second = true;
           break;
       }
    }
    return rt;
}

DynamicTable::DynamicTable()
    :m_maxDataSize(4 * 1024),
     m_dataSize(0){
}
int32_t DynamicTable::update(const std::string& name,const std::string& value){
    //这里加32 是规定
    int len = name.size() + value.size() + 32;
    while((len + m_dataSize) > m_maxDataSize && !m_datas.empty()){
        std::pair<std::string,std::string> & tmp = m_datas[0];
        m_dataSize -= (tmp.first.size() + tmp.second.size() + 32);
        m_datas.erase(m_datas.begin());
    }
    m_dataSize += len;
    m_datas.push_back({name,value});
    return 0;
}
//在动态表中找到对应的下标之后之所以要 + 62的原因就是[0,62]之间的下标数据静态表
int32_t DynamicTable::findIndex(const std::string& name){
    //首先在静态表中进行查找
    int32_t rt = GetStaticHeadersIdx(name);
    if(rt == -1){
        size_t len = m_datas.size() - 1;
        //如果在静态表中没找到，那么就到动态表中进行查找
        for(size_t i = 0;i < m_datas.size();++i){
           if(m_datas[len - i].first == name){
               rt = i + 62;
               break;
           }
        }
    }
    return rt;
}

std::pair<int32_t,bool> DynamicTable::findPair(const std::string& name,
                                                 const std::string& value)const{
    auto rt = GetStaticHeadersPair(name,value);
    if(!rt.second){
        //如果静态表中没有找到的话
        size_t len = m_datas.size() - 1;
        for(size_t i = 0;i <= len;++i){
            if(m_datas[len - i].first == name){
                if(rt.first == -1){
                    rt.first = i + 62;
                }
            }else{
                continue;
            }
            if(m_datas[len - i].second == value){
                rt.first = i + 62;
                rt.second = true;
                break;
            }
        }
    }
    return rt;
}
std::pair<std::string,std::string> DynamicTable::getPair(uint32_t idx)const{
    if(idx < 62){
        return GetStaticHeaders(idx);
    }
    //在动态表中
    idx -= 62;
    if(idx < (uint32_t)m_datas.size()){
        return m_datas[m_datas.size() - 1 - idx];
    }
    return std::make_pair("","");
}

std::string DynamicTable::getName(uint32_t idx)const{
    return getPair(idx).first;
}
std::string DynamicTable::toString()const{
    std::stringstream ss;
    ss << " [DynamicTable max_dataSize = " << m_maxDataSize
       << "  dataSize = " << m_dataSize 
       << " ]" << std::endl;
    int idx = 62;
    for(int i = m_datas.size() - 1;i >= 0;--i){
        ss << "\t" << idx++ << " : " << m_datas[idx].first << " - " << m_datas[i].second
           << std::endl;
    }
    return ss.str();
}
                                            


}
}
