#ifndef _JKYI_HPACK_H_
#define _JKYI_HPACK_H_

#include"JKYi/bytearray.h"
#include"dynamic_table.h"

namespace JKYi{
namespace http2{

//索引header 字段表示: 以 一位模式'1'开头
//字面header 字段表示: 
//1. 可以更新到动态表中的以'01'两位模式开头
//2. 不更新到动态表中的以'0000'四位模式开头
//3. 从不会更新到动态表中的以'0001'四位模式开头
enum class IndexType{
    INDEXED                       = 0, // headers的name和value都在表中
    WITH_INDEXING_INDEXED_NAME    = 1, // name在表中value需要编码传输,并且会更新到动态表中去
    WITH_INDEXING_NEW_NAME        = 2, // name和value都不在表中，所以都需要编码传输并且会更新到动态表中去
    WITHOUT_INDEXING_INDEXED_NAME = 3, // name 在表中，value需要编码传输并且不更新到动态表中去
    WITHOUT_INDEXING_NEW_NAME     = 4, // name和value都不在表中，都需要编码传输并且不更新到表中去
    NERVER_INDEXED_INDEXED_NAME   = 5, // name在表中，value需要编码传输，并永远不更新到动态表中去
    NERVER_INDEXED_NEW_NAME       = 6, // name和value都不在表中，需要编码传输并且永远不会更新到动态表中去
    ERROR                         = 7
};

std::string IndexTypeToString(IndexType type);

//字符编码
struct StringHeader{
    union{
        struct {
            uint8_t len : 7;
            uint8_t h : 1;
        };
        uint8_t h_len;
    };
};
//struct FieldHeader{
//    union{
//        struct {
//            uint8_t index : 7;
//            uint8_t code : 1;
//        }indexed;
//        struct {
//            uint8_t index : 6;
//            uint8_t code : 2;
//        }with_indexing;
//        struct {
//            uint8_t index : 4;
//            uint8_t code : 4;
//        }other;
//        uint8_t type = 0;
//    };
//};

struct HeaderField{
    IndexType type = IndexType::ERROR;
    bool h_name = 0;
    bool h_value = 0;
    uint32_t index = 0;
    std::string name;
    std::string value;

    std::string toString()const;
};

//HPACK算法是http2.0中专门用来对报文首部进行压缩的算法
class HPack{
public:
    typedef std::shared_ptr<HPack> ptr;
    HPack(DynamicTable& table);

    int parse(ByteArray::ptr ba,int length);
    int parse(std::string & data);
    int pack(HeaderField * header,ByteArray::ptr ba);
    int pack(const std::vector<std::pair<std::string,std::string>>&headers,
              ByteArray::ptr ba);
    int pack(const std::vector<std::pair<std::string,std::string>>&headers,
              std::string& out);

    const std::vector<HeaderField>& getHeaders()const { return m_headers; }
    static int Pack(HeaderField * header,ByteArray::ptr ba);

    std::string toString()const;
public:
    static int WriteVarInt(ByteArray::ptr ba,int32_t prefix,uint64_t value,
                           uint8_t flag);
    static uint64_t ReadVarInt(ByteArray::ptr ba,int32_t prefix);
    static uint64_t ReadVarInt(ByteArray::ptr ba,uint8_t b,int32_t prefix);
    static std::string ReadString(ByteArray::ptr ba);
    static int WriteString(ByteArray::ptr ba,const std::string& str,bool h);
private:
    std::vector<HeaderField> m_headers;
    //静态表和动态表
    DynamicTable& m_table;
};


}
}
#endif
