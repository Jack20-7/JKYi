#ifndef _JKYI_STREAM_H_
#define _JKYI_STREAM_H_

#include<memory>
#include"bytearray.h"

namespace JKYi{

//抽象类，提供公共的接口
class Stream{
public:
    typedef std::shared_ptr<Stream> ptr;

    virtual ~Stream(){}
    
    virtual int read(void * buffer,size_t length) = 0;
    virtual int read(ByteArray::ptr ba,size_t length) = 0;
    //读取固定长度的字节数
    virtual int readFixSize(void * buffer,size_t length);
    virtual int readFixSize(ByteArray::ptr ba,size_t length);

    virtual int write(const void * buffer,size_t length) = 0;
    virtual int write(ByteArray::ptr ba,size_t length) = 0;
    virtual int writeFixSize(const void * buffer,size_t length);
    virtual int writeFixSize(ByteArray::ptr ba,size_t length);

    virtual void close() = 0;
};
}
#endif
