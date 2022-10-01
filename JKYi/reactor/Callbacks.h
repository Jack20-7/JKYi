#ifndef _JKYI_CALLBACKS_H_
#define _JKYI_CALLBACKS_H_

#include"JKYi/timestamp.h"
#include"JKYi/Types.h"

#include<functional>
#include<memory>

namespace JKYi{

using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;

template<class T>
inline T* get_pointer(const std::shared_ptr<T>& ptr){
    return ptr.get();
}

template<class T>
inline T* get_pointer(const std::unique_ptr<T>& ptr){
    return ptr.get();
}

template<typename To, typename From>
inline ::std::shared_ptr<To> down_pointer_cast(const ::std::shared_ptr<From>& f) {
  if (false)
  {
    implicit_cast<From*, To*>(0);
  }

#ifndef NDEBUG
  assert(f == NULL || dynamic_cast<To*>(get_pointer(f)) != NULL);
#endif
  return ::std::static_pointer_cast<To>(f);
}

namespace net{

class Buffer;
class TcpConnection;
typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;

typedef std::function<void ()> TimerCallback;

typedef std::function<void (const TcpConnectionPtr&)> ConnectionCallback;
typedef std::function<void (const TcpConnectionPtr&)> CloseCallback;
typedef std::function<void (const TcpConnectionPtr&)> WriteCompleteCallback;
typedef std::function<void (const TcpConnectionPtr&,size_t)> HighWaterMarkCallback;

typedef std::function<void (const TcpConnectionPtr&,Buffer*,
                                       Timestamp)> MessageCallback;

void defaultConnectionCallback(const TcpConnectionPtr& conn);

void defaultMessageCallback(const TcpConnectionPtr& conn,Buffer*,
                                Timestamp receiveTime);
}//namespace net

}


#endif
