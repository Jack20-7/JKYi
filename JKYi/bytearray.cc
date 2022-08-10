#include"bytearray.h"

#include<fstream>
#include<sstream>
#include<string.h>
#include<iomanip>

#include"log.h"
#include"endian.h"

namespace JKYi{

static Logger::ptr g_logger=JKYI_LOG_NAME("system");

//首先初始化链表节点

ByteArray::Node::Node(size_t s)
    :ptr(new char[s]),
     size(s),
     next(nullptr){

}

ByteArray::Node::Node()
   :ptr(nullptr),
    size(0),
    next(nullptr){
  
}

ByteArray::Node::~Node(){
    if(ptr){
      delete [] ptr;
    }
    ptr=nullptr;
}

ByteArray::ByteArray(size_t base_size)
    :m_baseSize(base_size)
    ,m_position(0)
    ,m_capacity(base_size)
    ,m_size(0)
    ,m_endian(JKYI_BIG_ENDIAN)
    ,m_root(new Node(base_size))
    ,m_cur(m_root){

} 

ByteArray::~ByteArray(){
    Node* tmp=m_root;
    while(tmp){
       m_cur=tmp;
       tmp=tmp->next;
       delete m_cur;
    }
}

bool ByteArray::isLittleEndian()const{

    return m_endian==JKYI_LITTLE_ENDIAN;
}

void ByteArray::setIsLittleEndian(bool v){
    if(v){
        m_endian=JKYI_LITTLE_ENDIAN;
    }else{
        m_endian=JKYI_BIG_ENDIAN;
    }
}

//
void ByteArray::writeFint8(int8_t val){
    write(&val,sizeof(val));
}
void ByteArray::writeFuint8(uint8_t val){
    write(&val,sizeof(val));
}

void ByteArray::writeFint16(int16_t val){
     if(m_endian!= JKYI_BYTE_ORDER){
       val=byteswap(val);
     } 
     write(&val,sizeof(val));
}
void ByteArray::writeFuint16(uint16_t val){
    if(m_endian != JKYI_BYTE_ORDER){
        val=byteswap(val);
    }
     write(&val,sizeof(val));
}

void ByteArray::writeFint32(int32_t val){
    if(m_endian != JKYI_BYTE_ORDER){
        val=byteswap(val);
    }
    write(&val,sizeof(val));
}
void ByteArray::writeFuint32(uint32_t val){
    if(m_endian != JKYI_BYTE_ORDER){
        val=byteswap(val);
    }
    write(&val,sizeof(val));
} 

void ByteArray::writeFint64(int64_t val){
    if(m_endian != JKYI_BYTE_ORDER){
        val=byteswap(val);
    } 
    write(&val,sizeof(val));
} 
void ByteArray::writeFuint64(uint64_t val){
    if(m_endian != JKYI_BYTE_ORDER){
        val=byteswap(val);
    }
    write(&val,sizeof(val));
}

//该函数用来将32位的有符号整形转化为无符号整形
static uint32_t EncodeZigzag32(const int32_t &val){
   if( val<0 ){
       //如果是一个负数的话
       return ((uint32_t)(-val))*2-1;
   }else{
       return val*2;
   }
}
static uint64_t EncodeZigzag64(const int64_t &val){
    if(val<0){
        return ((uint64_t)(-val))*2-1;
    }else{
        return val*2;
    }
}

//该函数用来将数据从无符号转换为有符号
static int32_t DecodeZigzag32(const uint32_t&val){
    return (val >> 1) ^ -(val & 1);
}
static int64_t DecodeZigzag64(const uint64_t &val){
    return (val >> 1) ^ -(val & 1);
}

//采用protobuf中的varint压缩算法对写入的数据进行压缩
void ByteArray::writeInt32(int32_t val){
    writeUint32(EncodeZigzag32(val));

}
void ByteArray::writeUint32(uint32_t val){
    uint8_t tmp[5];
    uint8_t i=0;
    while(val >= 0x80){
        tmp[i++]= (val & 0x7f) | 0x80;
        val >>= 7;
    }
    tmp[i++]=val;
    write(tmp,i);
}

void ByteArray::writeInt64(int64_t val){
    writeUint64(EncodeZigzag64(val));

}
void ByteArray::writeUint64(uint64_t val){

    uint8_t tmp[10];
    uint8_t i=0;
    while(val >= 0x80){
      tmp[i++]= (val & 0x7f) | 0x80;
      val >>= 7;
    }
    tmp[i++]=val;
    write(tmp,i);
}

void ByteArray::writeFloat(float value){
    uint32_t val;
    memcpy(&val,&value,sizeof(value));
    writeFuint32(val);
}
void ByteArray::writeDouble(double value){
    uint64_t val;
    memcpy(&val,&value,sizeof(value));
    writeFuint64(val);
}

//下面是写入字符串

void ByteArray::writeStringF16(const std::string& value){
    //首先写入长度
    writeFuint16(value.size());
    //再写入字符
    write(value.c_str(),value.size());
}
void ByteArray::writeStringF32(const std::string& value){
    writeFuint32(value.size());
    write(value.c_str(),value.size());
}
void ByteArray::writeStringF64(const std::string& value){
    writeFuint64(value.size());
    write(value.c_str(),value.size());
}

void ByteArray::writeStringVint(const std::string& value){
    writeUint64(value.size());
    write(value.c_str(),value.size());
}

void ByteArray::writeStringWithoutLength(const std::string& value){
    write(value.c_str(),value.size());
}

//下面是提供的读接口

int8_t ByteArray::readFint8(){
    int8_t val;
    read(&val,sizeof(val));
    return val;
}
uint8_t ByteArray::readFuint8(){
    uint8_t val;
    read(&val,sizeof(val));
    return val;
}
#define XX(type) \
 {\
    type val;\
    read(&val,sizeof(val));\
    if(m_endian != JKYI_BYTE_ORDER){\
        return byteswap(val);\
    }else{ \
        return val;\
    }\
 }

int16_t ByteArray::readFint16(){
     XX(int16_t);
}
uint16_t ByteArray::readFuint16(){
    XX(uint16_t);
}
int32_t ByteArray::readFint32(){
    XX(int32_t);
}
uint32_t ByteArray::readFuint32(){
    XX(uint32_t);
}
int64_t ByteArray::readFint64(){
    XX(int64_t);
}
uint64_t ByteArray::readFuint64(){
    XX(uint64_t);
}

#undef XX

//读取压缩过的数据.涉及到解压缩
int32_t ByteArray::readInt32(){
    return DecodeZigzag32(readUint32());
}
uint32_t ByteArray::readUint32(){
   uint32_t result=0; 
   for(int i=0;i<32;i+=7){
       uint8_t val=readFuint8();
       if(val < 0x80){
           //如果是最后一个有效字节的话
           result |= ((uint32_t)val) << i;
           break;
       }else{
           result |= ((uint32_t)(val & 0x7f) << i);
       }
   }
   return result;
}
int64_t ByteArray::readInt64(){
   return DecodeZigzag64(readUint64());
}
uint64_t ByteArray::readUint64(){
    uint64_t result=0;
    for(int i=0;i<64;i+=7){
        uint8_t val=readFuint8();
        if(val < 0x80){
            result |= ((uint64_t)val) << i;
            break;
        }else{
            result |= ((uint64_t)(val & 0x7f) << i);
        }
    }
    return result;
}

float ByteArray::readFloat(){
   uint32_t value=readFuint32();
   float ret;
   memcpy(&ret,&value,sizeof(value));
   return ret;
}
double ByteArray::readDouble(){
    uint64_t value=readFuint64();
    double ret;
    memcpy(&ret,&value,sizeof(value));
    return ret;
}

//读取字符串

std::string ByteArray::readStringF16(){
    uint16_t len=readFuint16();
    std::string buff;
    buff.resize(len);
    read(&buff[0],len);
    return buff;
}
std::string ByteArray::readStringF32(){
    uint32_t len=readFuint32();
    std::string buff;
    buff.resize(len);
    read(&buff[0],len);
    return buff;
}
std::string ByteArray::readStringF64(){
    uint64_t len=readFuint64();
    std::string buff;
    buff.resize(len);
    read(&buff[0],len);
    return buff;
}
std::string ByteArray::readStringVint(){
    uint64_t len=readUint64();
    std::string buff;
    buff.resize(len);
    read(&buff[0],len);
    return buff;
}

//将ByteArray恢复到创建态 
void ByteArray::clear(){
   m_position=0;
   m_size=0;
   m_capacity=m_baseSize;

   Node* tmp=m_root->next;
   while(tmp){
       m_cur=tmp->next;
       delete tmp;
       tmp=m_cur;
   }
   m_cur=m_root;
   m_root->next=nullptr;
}

//正在向ByteArray中写入数据的函数
void ByteArray::write(const void * buf,size_t len){
    if(len==0){
        return ;
    }
    //
    addCapacity(len);

    //表示的是当前操作位置相较于当前节点的偏移量
    size_t npos=m_position % m_baseSize;

    //当前所在节点的容量
    size_t ncap=m_cur->size-npos;

    //当前已经写入的字节数
    size_t bpos=0;

    while(len > 0){
        if(ncap >= len){
            //表示当前节点就能够容纳剩余的需要写入的数据
            memcpy(m_cur->ptr+npos,(const char*)buf+bpos,len);
            if(m_cur->size == (npos+len)){
                m_cur=m_cur->next;
            }
            m_position+=len;
            bpos+=len;
            len=0;
        }else{
            memcpy(m_cur->ptr+npos,(const char*)buf+bpos,ncap);
            m_position+=ncap; 
            bpos+=ncap;
            len-=ncap;
            m_cur=m_cur->next;
            ncap=m_cur->size;
            npos=0;
        }
    }

    if(m_position > m_size){
        m_size=m_position;
    }
}

//正在读数据的函数
void ByteArray::read(void * buf,size_t len){
   //如果没有那么多数据要读
   if(len > getReadSize()){
      throw std::out_of_range("not enough data to read");
   }
   //
   size_t npos=m_position % m_baseSize;
   size_t ncap=m_cur->size - npos;
   size_t bpos=0;

   while(len > 0){
       if(ncap >= len){
           memcpy((char*)buf+bpos,m_cur->ptr+npos,len);
           if(m_cur->size == (len + npos)){
               m_cur=m_cur->next;
           }
           m_position+=len;
           bpos+=len;
           len=0;
       }else{
           memcpy((char*)buf+bpos,m_cur->ptr+npos,ncap);
           m_position+=ncap;
           bpos+=ncap;
           len-=ncap;
           m_cur=m_cur->next;
           ncap=m_cur->size;
           npos=0;
       }
   }
}
//从指定位置读取数据
void ByteArray::read(void *buf,size_t len,size_t position)const{
    if(len > (m_size - position)){
        throw std::out_of_range("not enough data to read");
    }
    //
    size_t npos=position % m_baseSize;
    size_t ncap=m_cur->size - npos;
    size_t bpos=0;
    Node * cur=m_cur;
    while(len > 0){
        if(ncap >= len){
            memcpy((char*)buf+bpos,cur->ptr+npos,len);
            if(cur->size == (npos + len)){
               cur=cur->next;
            }
            position+=len;
            bpos+=len;
            len=0;
        }else{
            memcpy((char*)buf+bpos,cur->ptr+npos,ncap);
            position+=ncap;
            bpos+=ncap;
            len-=ncap;
            cur=cur->next;
            ncap=cur->size;
            npos=0;
        }
    }

    return ;
} 

void ByteArray::setPosition(size_t v){
  if(v > m_capacity){
    throw std::out_of_range("setPosition out of range");
  }
  m_position=v;
  if(m_position > m_size){
      m_size=m_position;
  }
  //然后调整m_cur的位置
  m_cur=m_root;
  while(v > m_cur->size){
     v-=m_cur->size;
     m_cur=m_cur->next;
  }
  if(v == m_cur->size){
      m_cur=m_cur->next;
  }
}

//将ByteArray中[m_position,m_size)中的数据写入文件
bool ByteArray::writeToFile(const std::string& name)const{
    std::ofstream ofs;
    ofs.open(name,std::ios::trunc | std::ios::binary);
    if(!ofs){
        JKYI_LOG_ERROR(g_logger)<<"writeToFile name="<<name
                                <<" error,errno="<<errno
                                <<" errstr="<<strerror(errno);
        return false;
    }
    //当前还能够读取的数据大小
    int64_t read_size=getReadSize();
    
    //当前的位置
    int64_t pos=m_position;

    Node* cur=m_cur;

    while(read_size > 0){
        int diff=pos % m_baseSize;
        int64_t len=(read_size > (int64_t)m_baseSize? m_baseSize:read_size)-diff;
        ofs.write(cur->ptr+diff,len);
        cur=cur->next;
        pos+=len;
        read_size-=len;
    }
    return true;
}

bool ByteArray::readFromFile(const std::string& name){
    std::ifstream ifs;
    ifs.open(name,std::ios::binary);
    if(!ifs){
        JKYI_LOG_ERROR(g_logger)<<"readFromFile name="<<name
                                <<" error,errno="<<errno
                                <<" errstr="<<strerror(errno);
        return false;
    }
    std::shared_ptr<char>buff(new char[m_baseSize],[](char * ptr){ delete [] ptr;});
    while(!ifs.eof()){
        ifs.read(buff.get(),m_baseSize);
        write(buff.get(),ifs.gcount());
    }
    return true;
}

//该函数的真实意思是保证有size大小的可写入空间
void ByteArray::addCapacity(size_t size){
    if(size == 0){
        return ;
    }
    size_t old_cap=getWriteSize();
    if(old_cap >= size){
        return ;
    }
    size=size-old_cap;
    //需要增加的节点个数
    size_t count=ceil(1.0 * size / m_baseSize);

    Node * tmp=m_root;
    while(tmp->next){
        tmp=tmp->next;
    }

    //用来记录增加的第一个节点的位置
    Node * first = nullptr;
    for(size_t i=0;i<count;++i){
      tmp->next=new Node(m_baseSize);
      if(first==nullptr){
          first=tmp->next;
      } 
      tmp=tmp->next;
      m_capacity+=m_baseSize;
    }
    if(old_cap == 0){
       m_cur=first;
    } 
}

std::string ByteArray::toString()const{
   std::string str;
   str.resize(getReadSize());
   if(str.empty()){
       return str;
   } 
   read(&str[0],str.size(),m_position);
   return str;
}
std::string ByteArray::toHexString()const{
    std::string str=toString();
    std::stringstream ss;

    for(size_t i=0;i<str.size();++i){
        //每32位一组
        if(i > 0 && i % 32 == 0){
            ss<<std::endl;
        }
        ss<<std::setw(2)<<std::setfill('0') <<std::hex
          <<(int)(uint8_t)str[i]<<" ";
    }
    return ss.str();
}

uint64_t ByteArray::getReadBuffers(std::vector<iovec>&buffers,uint64_t len)const{
    len=len > getReadSize() ? getReadSize():len;
    if(len == 0){
        return 0;
    }

    uint64_t res=len;

    size_t npos=m_position % m_baseSize;
    size_t ncap=m_cur->size - npos;
    struct iovec iov;

    Node * cur=m_cur;
    while(len > 0){
        if(ncap >= len){
            iov.iov_base=cur->ptr+npos;
            iov.iov_len=len;
            len=0;
        }else{
            iov.iov_base=cur->ptr+npos;
            iov.iov_len=ncap;
            len-=ncap;
            cur=cur->next;
            ncap=cur->size;
            npos=0;
        }
        buffers.push_back(iov);
    }
    return res;
}

uint64_t ByteArray::getReadBuffers(std::vector<iovec>&buffers,uint64_t len,uint64_t position)const{
    len=(len > (m_size - position)) ? (m_size-position) : len;
    if(len == 0){
        return 0;
    }
    uint64_t res=len;

    size_t npos=position % m_baseSize;
    size_t count=position / m_baseSize;
    Node * cur=m_root;

    while(count > 0){
       cur=cur->next;
       count--;
    }
    size_t ncap=cur->size - npos;
    struct iovec iov;

    while(len > 0){
        if(ncap >= len){
            iov.iov_base=cur->ptr+npos;
            iov.iov_len=len;
            len=0;
        }else{
            iov.iov_base=cur->ptr+npos;
            iov.iov_len=ncap;
            len-=ncap;
            cur=cur->next;
            ncap=cur->size;
            npos=0;
        }
        buffers.push_back(iov);
    }
    return res;
}

//将m_position后面len长度的可写空间已vector<iovec>的形式返回，这样方便socket进行操作
uint64_t ByteArray::getWriteBuffers(std::vector<iovec>&buffers,uint64_t len){
    if(len == 0){
        return 0;
    }
    //保证有len大小的可写空间
    addCapacity(len);

    uint64_t res=len;

    size_t npos=m_position % m_baseSize;
    size_t ncap=m_cur->size - npos;
    struct iovec iov;

    Node * cur=m_cur;
    while(len > 0){
       if(ncap >= len){
           iov.iov_base=cur->ptr+npos;
           iov.iov_len=len;
           len=0;
       }else{
           iov.iov_base=cur->ptr+npos;
           iov.iov_len=ncap;
           
           len-=ncap;
           cur=cur->next;
           ncap=cur->size;
           npos=0;
       }
       buffers.push_back(iov);
    } 
    return res;
}












}
