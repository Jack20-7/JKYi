#ifndef JKYI_DYNAMIC_TABLE_H_
#define JKYI_DYNAMIC_TABLE_H_

#include<vector>
#include<string>

namespace JKYi{
namespace http2{

//在http2.0中用来对http报文首部字段进行编码
class DynamicTable{
public:
    DynamicTable();
    ~DynamicTable();

    int32_t update(const std::string& name,const std::string& value);
    int32_t findIndex(const std::string& name);
    std::pair<int32_t,bool> findPair(const std::string& name,const std::string& value)const;
    std::pair<std::string,std::string> getPair(uint32_t idx)const;
    std::string getName(uint32_t idx)const;
    std::string toString()const;

    void setMaxDataSize(int32_t v) { m_maxDataSize = v; }
public:
    static std::pair<std::string,std::string> GetStaticHeaders(uint32_t idx);
    static int32_t GetStaticHeadersIdx(const std::string& name);
    static std::pair<int32_t,bool> GetStaticHeadersPair(const std::string& name,const std::string& val);
private:
    int32_t m_maxDataSize;
    int32_t m_dataSize;
    std::vector<std::pair<std::string,std::string>>m_datas;
};
}
}

#endif
