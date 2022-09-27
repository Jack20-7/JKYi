#ifndef _JKYI_BUFFER_H_
#define _JKYI_BUFFER_H_

#include"JKYi/copyable.h"
#include"JKYi/endian.h"
#include"JKYi/macro.h"
#include"JKYi/reactor/StringPiece.h"

#include<algorithm>
#include<vector>
#include<string.h>

namespace JKYi{
namespace net{

class Buffer{
public:
    static const size_t kCheapPrepend = 8;           //buffer中预留的头部的大小
    static const size_t kInitialSize = 1024;         //buffer默认的大小

    explicit Buffer(size_t initialSize = kInitialSize)
        :buffer_(kCheapPrepend + initialSize),
         readerIndex_(kCheapPrepend),
         writerIndex_(kCheapPrepend){

        JKYI_ASSERT(readableBytes() == 0);
        JKYI_ASSERT(writableBytes() == initialSize);
        JKYI_ASSERT(prependableBytes() == kCheapPrepend);
     }

    void swap(Buffer& rhv){
        buffer_.swap(rhv.buffer_);
        std::swap(readerIndex_,rhv.readerIndex_);
        std::swap(writerIndex_,rhv.writerIndex_);
    }
    size_t readableBytes()const{
        return writerIndex_ - readerIndex_;
    }
    size_t writableBytes()const{
        return buffer_.size() - writerIndex_;
    }
    size_t prependableBytes()const{
        return readerIndex_;
    }

    //返回第一个可以读取的字节
    const char * peek()const{
        return begin() + readerIndex_;
    }
    //在可以读取的字节中查找'/r/n'
    const char * findCRLF()const{
        const char * crlf = std::search(peek(),beginWrite(),kCRLF,kCRLF + 2);
        return crlf == beginWrite() ? NULL : crlf;
    }
    const char * findCRLF(const char * start)const{
        JKYI_ASSERT(start >= peek());
        JKYI_ASSERT(start <= beginWrite());

        const char * crlf = std::search(start,beginWrite(),kCRLF,kCRLF + 2);
        return crlf == beginWrite() ? NULL : crlf;
    }
    //寻找/n
    const char * findEOL()const{
      const void * eol = memchr(peek(),'\n',readableBytes());
      return static_cast<const char*>(eol);
    }
    const char * findEOL(const char * start)const{
      JKYI_ASSERT(start >= peek());
      JKYI_ASSERT(start <= beginWrite());

      const void * eol = memchr(start,'\n',beginWrite() - start); 
      return static_cast<const char *>(eol);
    }
    //移动readIndex_
    void retrieve(size_t len){
        JKYI_ASSERT(len <= readableBytes());
        if(len < readableBytes()){
           readerIndex_ += len;
        }else{
            retrieveAll();
        }
    }
    void retrieveAll(){
        readerIndex_ = kCheapPrepend;
        writerIndex_ = kCheapPrepend;
    }
    //将readIndex_ 移动到 end
    void retrieveUntil(const char * end){
        JKYI_ASSERT(peek() <= end);
        JKYI_ASSERT(end <= beginWrite());
        retrieve(end - peek());
    }
    void retrieveInt64(){
        return retrieve(sizeof(int64_t));
    }
    void retrieveInt32(){
        return retrieve(sizeof(int32_t));
    }
    void retrieveInt16(){
        return retrieve(sizeof(int16_t));
    }
    void retrieveInt8(){
        return retrieve(sizeof(int8_t));
    }
    std::string retrieveAllAsString(){
        return retrieveAsString(readableBytes());
    }

    std::string retrieveAsString(size_t len){
        JKYI_ASSERT(len <= readableBytes());
        std::string ret(peek(),len);
        retrieve(len);
        return ret;
    }
    StringPiece toStringPiece()const{
        return StringPiece(peek(),static_cast<int>(readableBytes()));
    }
    void append(const StringPiece& str){
        append(str.data(),str.size());
    }

    void append(const char * data,size_t len){
        //确保有这么多空间可以写
        ensureWritableBytes(len);
        std::copy(data,data + len,beginWrite());
        //更新writerIndex_
        hasWritten(len);
    }
    void append(const void * data,size_t len){
        return append(static_cast<const char *>(data),len);
    }
    //确保buffer中有这么多的可用空间
    void ensureWritableBytes(size_t len){
        if(writableBytes() < len){
            makeSpace(len);
        }
        JKYI_ASSERT(writableBytes() >= len);
    }
    char * beginWrite(){
        return begin() + writerIndex_;
    }
    const char * beginWrite()const{
        return begin() + writerIndex_;
    }
    void hasWritten(size_t len){
        JKYI_ASSERT(writableBytes() >= len);
        writerIndex_ += len;
    }
    void unwrite(size_t len){
        JKYI_ASSERT(readableBytes() >= len);
        writerIndex_ -= len;
    }
    //append
    void appendInt64(int64_t x){
        int64_t be64 = JKYi::toNetEndian(x);
        append(&be64,sizeof(be64));
    }
    void appendInt32(int32_t x){
        int32_t be32 = JKYi::toNetEndian(x);
        append(&be32,sizeof(be32));
    }
    void appendInt16(int16_t x){
        int16_t be16 = JKYi::toNetEndian(x);
        append(&be16,sizeof(be16));
    }
    void appendInt8(int8_t x){
        int8_t be8 = x;
        append(&be8,sizeof(be8));
    }
    //read
    int64_t readInt64(){
        int64_t result = peekInt64();
        retrieveInt64();
        return result;
    }
    int32_t readInt32(){
        int32_t result = peekInt32();
        retrieveInt32();
        return result;
    }
    int16_t readInt16(){
        int16_t result = peekInt16();
        retrieveInt16();
        return result;
    }
    int8_t readInt8(){
        int8_t result = peekInt8();
        retrieveInt8();
        return result;
    }
    //peekIntx
    int64_t peekInt64()const{
        JKYI_ASSERT(readableBytes() >= sizeof(int64_t));
        int64_t be64 = 0;
        ::memcpy(&be64,peek(),sizeof(be64));
        return JKYi::toLittleEndian(be64);
    }
    int32_t peekInt32()const{
        JKYI_ASSERT(readableBytes() >= sizeof(int32_t));
        int32_t be32 = 0;
        ::memcpy(&be32,peek(),sizeof(be32));
        return JKYi::toLittleEndian(be32);
    }
    int16_t peekInt16()const{
        JKYI_ASSERT(readableBytes() >= sizeof(int16_t));
        int16_t be16 = 0;
        ::memcpy(&be16,peek(),sizeof(be16));
        return JKYi::toLittleEndian(be16);
    }
    int8_t peekInt8()const{
        JKYI_ASSERT(readableBytes() >= sizeof(int8_t));
        int8_t be8 = *peek();
        return be8;
    }
    //
    //prepend  头插函数
    void prependInt64(int64_t x){
        int64_t be64 = JKYi::toNetEndian(x);
        prepend(&be64,sizeof(be64));
    }
    void prependInt32(int32_t x){
        int32_t be32 = JKYi::toNetEndian(x);
        prepend(&be32,sizeof(be32));
    }
    void prependInt16(int16_t x){
        int16_t be16 = JKYi::toNetEndian(x);
        return prepend(&be16,sizeof(be16));
    }
    void prependInt8(int8_t x){
        int8_t be8 = x;
        prepend(&be8,sizeof(be8));
    }
    void prepend(const void * data,size_t len){
        JKYI_ASSERT(prependableBytes() >= len);
        readerIndex_ -= len;
        const char * d = static_cast<const char * >(data);
        std::copy(d,d + len,begin() + readerIndex_);
    }
    //
    void shrink(size_t reserve){
        Buffer other;
        other.ensureWritableBytes(readableBytes() + reserve);
        other.append(toStringPiece());
        swap(other);
    }

    size_t internalCapacity()const{
        return buffer_.capacity();
    }
    ssize_t readFd(int fd,int* savedErrno);
private:
    char* begin(){
        return &*buffer_.begin();
    }
    const char* begin()const{
        return &*buffer_.begin();
    }

    void makeSpace(size_t len){
        if(writableBytes() + prependableBytes() < len + kCheapPrepend){
            //如果移动没有用的话
            buffer_.resize(writerIndex_ + len);
        }else{
            JKYI_ASSERT(readerIndex_ > kCheapPrepend);
            size_t readable = readableBytes();
            std::copy(begin() + readerIndex_,
                      begin() + writerIndex_,
                      begin() + kCheapPrepend);
            readerIndex_ = kCheapPrepend;
            writerIndex_ = kCheapPrepend + readable;
            JKYI_ASSERT(readableBytes() == readable);
        }
        return ;
    }

private:
    std::vector<char> buffer_;
    size_t readerIndex_;            //当前可以读的第一个字节
    size_t writerIndex_;            //当前可以写的第一个字节

    static const char kCRLF[];      //存储换行标志
};

}
}



#endif
