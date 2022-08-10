#include"/home/admin/workSpace/JKYi/JKYi.h"

#include<iostream>

static JKYi::Logger::ptr g_logger=JKYI_LOG_ROOT();
void test(){
#define XX(type,len,write_fun,read_fun,base_size)\
  {\
      std::vector<type>vec;\
      for(int i=0;i<len;++i){\
         vec.push_back(rand());\
      }\
      JKYi::ByteArray::ptr ba(new JKYi::ByteArray(base_size));\
      for(auto& i : vec){\
          ba->write_fun(i);\
      }\
      ba->setPosition(0);\
      for(size_t i=0;i<vec.size();++i){\
          type v=ba->read_fun();\
          JKYI_ASSERT(v == vec[i]);\
      }\
      JKYI_ASSERT(ba->getReadSize()==0);\
      JKYI_LOG_INFO(g_logger)<< #write_fun"/"#read_fun\
                             << " ("#type") len="<<len\
                             <<" base_len="<<base_size\
                             <<" size="<<ba->getSize();\
  }
  
  XX(int8_t,100,writeFint8,readFint8,1);
  XX(uint8_t,100,writeFuint8,readFuint8,1);
  XX(int16_t,100,writeFint16,readFint16,1);
  XX(uint16_t,100,writeFuint16,readFuint16,1);
  XX(int32_t,100,writeFint32,readFint32,1);
  XX(uint32_t,100,writeFuint32,readFuint32,1);
  XX(int64_t,100,writeFint64,readFint64,1);
  XX(uint64_t,100,writeFuint64,readFuint64,1);


  XX(int32_t,100,writeInt32,readInt32,1);
  XX(uint32_t,100,writeUint32,readUint32,1);
  XX(int64_t,100,writeInt64,readInt64,1);
  XX(uint64_t,100,writeUint64,readUint64,1);

#undef XX
    

  JKYI_LOG_INFO(g_logger)<<"test part two---------------------------------------------------";
#define XX(type,len,write_fun,read_fun,base_size)\
 {\
    std::vector<type>vec;\
    for(int i=0;i<len;++i){\
       vec.push_back(rand());\
    }\
     JKYi::ByteArray::ptr ba(new JKYi::ByteArray(base_size));\
     for(auto& i: vec){\
         ba->write_fun(i);\
     }\
     ba->setPosition(0);\
     for(size_t i=0;i<vec.size();++i){\
        type v=ba->read_fun();\
         JKYI_ASSERT(v == vec[i]);\
     }\
     JKYI_ASSERT(ba->getReadSize() == 0);\
     JKYI_LOG_INFO(g_logger)<< #write_fun"/"#read_fun\
                             << " ("#type") len="<<len\
                             <<" base_len="<<base_size\
                             <<" size="<<ba->getSize();\
    ba->setPosition(0); \
    JKYI_ASSERT(ba->writeToFile("/tmp/"#type"_" #len"-"#read_fun".dat"));\
    JKYi::ByteArray::ptr ba2(new JKYi::ByteArray(base_size*2));\
    JKYI_ASSERT(ba2->readFromFile("/tmp/"#type"_" #len"-"#read_fun".dat"));\
    ba2->setPosition(0);\
    JKYI_ASSERT(ba->toString() == ba2->toString());\
    JKYI_ASSERT(ba->getPosition() == 0);\
    JKYI_ASSERT(ba2->getPosition() == 0);\
 }
  XX(int8_t,100,writeFint8,readFint8,1);
  XX(uint8_t,100,writeFuint8,readFuint8,1);
  XX(int16_t,100,writeFint16,readFint16,1);
  XX(uint16_t,100,writeFuint16,readFuint16,1);
  XX(int32_t,100,writeFint32,readFint32,1);
  XX(uint32_t,100,writeFuint32,readFuint32,1);
  XX(int64_t,100,writeFint64,readFint64,1);
  XX(uint64_t,100,writeFuint64,readFuint64,1);

  XX(int32_t,100,writeInt32,readInt32,1);
  XX(uint32_t,100,writeUint32,readUint32,1);
  XX(int64_t,100,writeInt64,readInt64,1);
  XX(uint64_t,100,writeUint64,readUint64,1);

#undef XX
  return ;
}
int main(int argc,char ** argv){
    JKYi::IOManager iom;
    iom.schedule(test);
    return 0;
}
