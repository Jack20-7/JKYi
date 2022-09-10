#include"hpack.h"
#include"JKYi/log.h"
#include"huffman.h"

namespace JKYi{
namespace http2{

static Logger::ptr g_logger = JKYI_LOG_NAME("system");

static std::vector<std::string> s_index_type_strings = {
    "INDEXED",
    "WITH_INDEXING_INDEXED_NAME",
    "WITH_INDEXING_NEW_NAME",
    "WITHOUT_INDEXING_INDEXED_NAME",
    "WITHOUT_INDEXING_NEW_NAME",
    "NERVER_INDEXED_INDEXED_NAME",
    "NERVER_INDEXED_NEW_NAME",
    "ERROR"
};

std::string IndexTypeToString(IndexType type){
    uint8_t v = (uint8_t) type;
    if(v <= 8){
        return s_index_type_strings[v];
    }
    return "UNKNOW(" +  std::to_string((uint32_t)v) + ")";
}

std::string HeaderField::toString()const{
    std::stringstream ss;
    ss << "header type " << IndexTypeToString(type)
       << " h_name = " << h_name
       << " h_value = " << h_value
       << " index = " << index
       << " name = " << name
       << " value = " << value;

    return ss.str();
}
HPack::HPack(DynamicTable& table)
    :m_table(table){
}
//用来对整数进行编码
//prefix表示的是前缀的位数
//整数编码分为前缀 和 列表两部分
int HPack::WriteVarInt(ByteArray::ptr ba,int32_t prefix,uint64_t value,
                              uint8_t flags){
   size_t pos = ba->getPosition();
   uint64_t v = (1 << prefix) - 1;
   if(value < v){
       //如果要压缩的数据 < 2 * prefix - 1
       ba->writeFuint8(value | flags);
       return 1;
   }
   //如果数据 > 2 * prefix - 1
   ba->writeFuint8(v | flags);
   value -= v;
   while(value >= 128){
       ba->writeFuint8((0x8 | (value & 0x7f)));
       value >>= 7;
   }
   ba->writeFuint8(value);
   return ba->getPosition() - pos;
}
//对整数进行解码
uint64_t HPack::ReadVarInt(ByteArray::ptr ba,int32_t prefix){
    uint8_t b = ba->readFuint8();
    uint8_t v = (1 << prefix) - 1;
    b &= v;
    if(b < v){
        return b;
    }
    uint64_t iv = b;
    int m = 0;
    do{
        b = ba->readFuint8();
        iv += ((uint64_t)(b & 0x7f)) << m;
        m += 7;
    }while(b & 0x80);
    return iv;
}
//给出了前缀
uint64_t HPack::ReadVarInt(ByteArray::ptr ba,uint8_t b,int32_t prefix){
    uint8_t v = (1 << prefix) - 1;
    b &= v;
    if(b < v){
        return b;
    }
    uint64_t iv = v;
    int m = 0;
    do{
        b = ba->readFuint8();
        iv += ((uint64_t)(b & 0x7f)) << m;
        m += 7;
    }while(b & 0x80);
    return iv;
}

//对string进行编码
//string的个数为
//H   length
//string data
std::string HPack::ReadString(ByteArray::ptr ba){
    uint8_t type = ba->readFuint8();
    int len = ReadVarInt(ba,type,7);
    std::string data;
    if(len){
      data.resize(len);
      ba->read(&data[0],len);
      if(type & 0x80){
          std::string out;
          Huffman::DecodeString(data,out);
          return out;
      }
    }
    return data;
}
int HPack::WriteString(ByteArray::ptr ba,const std::string& str,bool h){
    int pos = ba->getPosition();
    if(h){
        //如果要进行赫夫曼编码的话
        std::string new_str;
        Huffman::EncodeString(str,new_str,0);
        WriteVarInt(ba,7,new_str.size(),0x80);
        ba->write(new_str.c_str(),str.length());
    }else{
        WriteVarInt(ba,7,str.size(),0);
        ba->write(str.c_str(),str.length());
    }
    return ba->getPosition() - pos;
}
int HPack::parse(std::string& data){
    ByteArray::ptr ba(new ByteArray(&data[0],data.size(),false));
    return parse(ba,data.size());
}
int HPack::parse(ByteArray::ptr ba,int length){
    //记录已经解析的字节数
    int parsed = 0;
    int pos = ba->getPosition();
    while(parsed < length){
        HeaderField header;
        uint8_t type = ba->readFuint8();
        if(type & 0x80){
            //如果能够找到
            uint32_t idx = ReadVarInt(ba,type,7);
            header.type = IndexType::INDEXED;
            header.index = idx;
        }else{
            //如果不是name和value全都在表中的话
            if(type & 0x40){
                //带增量索引的header字段
                // 0 1 x x x x x x 
                uint32_t idx = ReadVarInt(ba,type,6);
                header.type = idx > 0 ? IndexType::WITH_INDEXING_INDEXED_NAME : 
                                        IndexType::WITH_INDEXING_NEW_NAME;
                header.index = idx;
            }else if((type & 0xF0) == 0){
                uint32_t idx = ReadVarInt(ba,type,4);
                header.type = idx > 0 ? IndexType::WITHOUT_INDEXING_INDEXED_NAME : 
                                        IndexType::WITHOUT_INDEXING_NEW_NAME;
                header.index = idx;
            }else if(type & 0x10){
                uint32_t idx = ReadVarInt(ba,type,4);
                header.type = idx > 0 ? IndexType::NERVER_INDEXED_INDEXED_NAME : 
                                        IndexType::NERVER_INDEXED_NEW_NAME;
                header.index = idx;
            }else{
                return -1;
            }

            if(header.index){
                header.value = ReadString(ba);
            }else{
                header.name = ReadString(ba);
                header.value = ReadString(ba);
            }
        }
        if(header.type == IndexType::INDEXED){
            std::pair<std::string,std::string> p = m_table.getPair(header.index);
            header.name = p.first;
            header.value = p.second;
        }else if(header.index > 0){
            std::pair<std::string,std::string> p = m_table.getPair(header.index);
            header.value = p.second;
        }

        //带增量的
        if(header.type == IndexType::WITH_INDEXING_INDEXED_NAME){
            m_table.update(m_table.getName(header.index),header.value);
        }else if(header.type == IndexType::WITH_INDEXING_NEW_NAME){
            m_table.update(header.name,header.value);
        }
        m_headers.emplace_back(std::move(header));
        parsed = ba->getPosition() - pos;
    }
    return parsed;
}
//对HeaderField进行写入
int HPack::Pack(HeaderField * header,ByteArray::ptr ba){
    int pos = ba->getPosition();
    if(header->type == IndexType::INDEXED){
       WriteVarInt(ba,7,header->index,0x80);
    }else if(header->type == IndexType::WITH_INDEXING_INDEXED_NAME){
       WriteVarInt(ba,6,header->index,0x40);
       WriteString(ba,header->value,header->h_value);
    }else if(header->type == IndexType::WITH_INDEXING_NEW_NAME){
       WriteVarInt(ba,6,header->index,0x40);
       WriteString(ba,header->name,header->h_name);
       WriteString(ba,header->value,header->h_value);
    }else if(header->type == IndexType::WITHOUT_INDEXING_INDEXED_NAME){
       WriteVarInt(ba,4,header->index,0x00);
       WriteString(ba,header->value,header->h_value);
    }else if(header->type == IndexType::WITHOUT_INDEXING_NEW_NAME){
       WriteVarInt(ba,4,header->index,0x00);
       WriteString(ba,header->name,header->h_name);
       WriteString(ba,header->value,header->h_value);
    }else if(header->type == IndexType::NERVER_INDEXED_INDEXED_NAME){
       WriteVarInt(ba,4,header->index,0x10);
       WriteString(ba,header->value,header->h_value);
    }else if(header->type == IndexType::NERVER_INDEXED_NEW_NAME){
       WriteVarInt(ba,4,header->index,0x10);
       WriteString(ba,header->name,header->h_name);
       WriteString(ba,header->value,header->h_value);
    }
    return ba->getPosition() - pos;
}
int HPack::pack(HeaderField * header,ByteArray::ptr ba){
    m_headers.push_back(*header);
    return Pack(header,ba);
}
int HPack::pack(const std::vector<std::pair<std::string,std::string>>& headers,
                  std::string& out){
    ByteArray::ptr ba(new ByteArray);
    int rt = pack(headers,ba);
    ba->setPosition(0);
    ba->toString().swap(out);
    return rt;
}
int HPack::pack(const std::vector<std::pair<std::string,std::string>>& headers,
                  ByteArray::ptr ba){
    //该函数的作用就是首先通过headers生成HeaderField，
    //然后在将数据放入到m_headers里面去并且写入ba
    int rt = 0;
    for(auto& i : headers){
        HeaderField h;
        auto p = m_table.findPair(i.first,i.second);
        if(p.second){
            //如果在表中name和value都能够找到
            h.type = IndexType::INDEXED;
            h.index = p.first;
        }else if(p.first){
            //name能够找到
            h.type = IndexType::WITH_INDEXING_INDEXED_NAME;
            h.index = p.first;
            h.h_value = Huffman::ShouldEncode(i.second);
            h.name = i.first;
            h.value = i.second;
            m_table.update(h.name,h.value);
        }else{
            h.type = IndexType::WITH_INDEXING_NEW_NAME;
            h.index = 0;
            h.h_name = Huffman::ShouldEncode(i.first);
            h.name = i.first;
            h.h_value = Huffman::ShouldEncode(i.second);
            h.value = i.second;
            m_table.update(h.name,h.value);
        }
        rt += pack(&h,ba);
    }
    return rt;
}

std::string HPack::toString()const{
    std::stringstream ss;
    ss << "[HPack size = " << m_headers.size() << "]" << std::endl;
    for(size_t i = 0;i < m_headers.size();++i){
        ss << "\t" << i << "\t:\t" << m_headers[i].toString() << std::endl;
    }
    ss << m_table.toString();
    return ss.str();
}

}
}
