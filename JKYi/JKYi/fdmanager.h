#ifndef _JKYI_FDMANAGER_H
#define _JKYI_FDMANAGER_H

#include<memory>
#include<vector>

#include"thread.h"
#include"singleton.h"

//封装一个fd的类，里面包含了文件描述符的相关信息

namespace JKYi{

class FdCtx:public std::enable_shared_from_this<FdCtx>{	
public:
   typedef std::shared_ptr<FdCtx> ptr;
   //
   FdCtx(int fd);

   ~FdCtx();

   bool isInit()const {return m_isInit;}

   bool isSocket()const{return m_isSocket;}

   bool isClose()const {return m_isClosed;}

   void setUserNonblock(bool flag){m_userNonblock=flag;}

   bool getUserNonblock()const {return m_userNonblock;}

   void setSysNonblock(bool flag) {m_sysNonblock=flag;}

   bool getSysNonblock()const {return m_sysNonblock;}

   void setTimeout(int type,uint64_t v);

   uint64_t getTimeout(int type);

private:
   bool init();

private:
  //下面这种bool类型成员变量的声明方式声明的成员只占用一个bit而不是1个字节 

  //是否被初始化
  bool m_isInit: 1;
  //是否是socket
  bool m_isSocket: 1;
  //是否是hook的非阻塞
  bool m_sysNonblock: 1;
  //是否是用户主动设置的非阻塞
  bool m_userNonblock: 1;
  //是否被关闭
  bool m_isClosed: 1;
  //管理的文件描述符
  int m_fd;
  //读超时时间
  uint64_t m_recvTimeout;
  //写超时时间
  uint64_t m_sendTimeout;
};   

//文件描述符上下文管理器
class FdManager{
public:
    typedef RWMutex RWMutexType;

	FdManager();

	~FdManager();
    //文件描述符返回对应的文件描述符上下文的结构体.第二个参数表示当没有时是否自动创建
	FdCtx::ptr get(int fd,bool auto_create=false);

	void del(int fd);
private:
    RWMutexType m_mutex; 

	std::vector<FdCtx::ptr>m_datas;
};
//使用单例类进行管理，这样要用到时就不需要自己手动的去创建对象
typedef Singleton<FdManager> FdMgr;

}

#endif
