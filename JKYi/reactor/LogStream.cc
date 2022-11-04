#include"JKYi/reactor/LogStream.h"

#include<algorithm>
#include<limits>
#include<type_traits>
#include<assert.h>
#include<string.h>
#include<stdint.h>
#include<stdio.h>

#if defined(__clang__)
#pragma clang diagnostic ignored "-Wtautological-compare"
#else
#pragma GCC diagnostic ignored "-Wtype-limits"
#endif

namespace JKYi{
namespace net{
namespace detail{

const char digits[] = "9876543210123456789";
const char* zero = digits + 9;
static_assert(sizeof(digits) == 20,"wrong number of digits");

const char digitsHex[] = "0123456789ABCDEF";
static_assert(sizeof(digitsHex) == 17,"wrong number of digitsHex");

//将传入的value转化为字符保存到buf里面去
template<class T>
size_t convert(char buf[],T value){
    T i = value;
    char* p = buf;
    do{
        int lsd = static_cast<int>(i % 10);
        i /= 10;
        *p++ = zero[lsd];
    }while(i != 0);

    if(value < 0){
        *p++ = '-';
    }
    *p = '\0';
    std::reverse(buf,p);

    return p - buf;
}

size_t convertHex(char buf[],uintptr_t value){
    uintptr_t i = value;
    char* p = buf;

    do{
        int lsd = static_cast<int>(i % 16);
        i /= 16;
        *p++ = digitsHex[lsd];
    }while( i != 0);

    *p = '\0';
    std::reverse(buf,p);

    return p - buf;
}

template class FixedBuffer<kSmallBuffer>;
template class FixedBuffer<kLargeBuffer>;
}//namespace detail

template<class T>
void LogStream::formatInteger(T v){
    if(buffer_.avail() >= kMaxNumericSize){
        size_t len = detail::convert(buffer_.current(),v);
        buffer_.add(len);
    }
}
LogStream& LogStream::operator<< (short v){
    *this << static_cast<int>(v);
    return *this;
}
LogStream& LogStream::operator<< (unsigned short v ){
    *this << static_cast<unsigned int>(v);
    return *this;
}
LogStream& LogStream::operator<< (int v){
    formatInteger(v);
    return *this;
}

LogStream& LogStream::operator<< (unsigned int v){
    formatInteger(v);
    return *this;
}
LogStream& LogStream::operator<< (long v){
    formatInteger(v);
    return *this;
}
LogStream& LogStream::operator<< (unsigned long v){
    formatInteger(v);
    return *this;
}
LogStream& LogStream::operator<< (long long v){
    formatInteger(v);
    return *this;
}
LogStream& LogStream::operator<< (unsigned long long v){
    formatInteger(v);
    return *this;
}
LogStream& LogStream::operator<< (const void * p){
    uintptr_t v = reinterpret_cast<uintptr_t>(p);
    if(buffer_.avail() >= kMaxNumericSize){
        char* buf = buffer_.current();
        buf[0] = '0';
        buf[1] = 'x';
        size_t len = detail::convertHex(buf + 2,v);
        buffer_.add(len + 2);
    }
    return *this;
}

LogStream& LogStream::operator<< (double v){
    if(buffer_.avail() >= kMaxNumericSize){
        int len = snprintf(buffer_.current(),kMaxNumericSize,"%.12g",v);
        buffer_.add(len);
    }
    return *this;
}

}//namespace net
} //namespace JKYi
