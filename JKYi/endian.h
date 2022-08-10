#ifndef _JKYI_ENDIAN_H_
#define _JKYI_ENDIAN_H_

//封装一些大端序-小端序之间进行转化的函数
#define JKYI_LITTLE_ENDIAN 1
#define JKYI_BIG_ENDIAN 2

#include<byteswap.h>
#include<stdint.h>

namespace JKYi{

  //8字节大小转化的函数
template<class T>
typename std::enable_if<sizeof(T)==sizeof(uint64_t),T>::type byteswap(T value){
	return (T)bswap_64((uint64_t)value);
}
//4字节转化的函数
template<class T>
typename std::enable_if<sizeof(T)==sizeof(uint32_t),T>::type byteswap(T value){
	return (T)bswap_32((uint32_t)value);
}

//2个字节的转化函数
template<class T>
typename std::enable_if<sizeof(T)==sizeof(uint16_t),T>::type byteswap(T value){
	return (T)bswap_16((uint16_t)value);
}
//以下通过系统提供的宏来根据当前系统是大端序还是小端序做出响应的处理
#if BYTE_ORDER == BIG_ENDIAN
#define JKYI_BYTE_ORDER JKYI_BIG_ENDIAN
#else
#define JKYI_BYTE_ORDER JKYI_LITTLE_ENDIAN
#endif

#if JKYI_BYTE_ENDIAN == JKYI_BIG_ENDIAN
//如果当前是大端序的话

template<class T>
T toNetEndian(T t){
	return t;
}

template<class T>
T toLittleEndian(T t){
	return byteswap(t);
}
#else 

//如果当前机器使用的是小端序的话

template<class T>
T toNetEndian(T t){
   return byteswap(t);
}

template<class T>
T toLittleEndian(T t){
	return t;
}

#endif

}
#endif
