#ifndef _JKYI_ZK_CLIENT_H_
#define _JKYI_ZK_CLIENT_H_

#include<memory>
#include<functional>
#include<string>
#include<stdint.h>
#include<vector>

#include"zookeeper/zookeeper.h"

namespace JKYi{

class ZKClient : public std::enable_shared_from_this<ZKClient>{
public:
    //监听器检测到的事件的类型
    class EventType{
    public:
        //节点创建事件
        static const int CREATED;
        //节点删除事件
        static const int DELETED;
        //节点数据改变事件
        static const int CHANGED;
        //子节点改变事件
        static const int CHILD;
        //客户端和服务器断开或重连时会触发
        static const int SESSION;
        //watch移除事件
        static const int NOWATCHING;
    };
    //创建的节点的类型
    class FlagsType{
    public:
        static const int EPHEMERAL;
        static const int SEQUENCE;
    };
    //建立的zookeeper连接的状态
    class StateType{
    public:
        static const int EXPIRED_SESSION;
        static const int AUTH_FAILED;
        static const int CONNECTING;
        static const int ASSOCIATING;
        static const int CONNECTED;
    };

    typedef std::shared_ptr<ZKClient> ptr;
    typedef std::function<void (int type,int stat,const std::string& path,ZKClient::ptr)>watcher_callback;
    //typedef void (* log_callback) (const char * message);

    ZKClient();
    ~ZKClient();

    bool init(const std::string& hosts,int recv_timeout,watcher_callback cb);
    //int32_t setServers(const std::string& hosts);
    //创建节点
    int32_t create(const std::string& path,const std::string& val,std::string& new_path,const struct ACL_vector* acl = &ZOO_OPEN_ACL_UNSAFE,int flags = 0);
    int32_t exists(const std::string& path,bool watch,Stat * stat = nullptr);
    int32_t del(const std::string& path,int version = -1);
    int32_t get(const std::string& path,std::string& val,bool watch,Stat * stat = nullptr);
    //int32_t getConfig(std::string& val,bool watch,Stat * stat = nullptr);
    int32_t set(const std::string& path,const std::string& val,int version = -1,Stat * stat = nullptr);
    int32_t getChildren(const std::string& path,std::vector<std::string>& val,bool watch,Stat * stat = nullptr);
    int32_t close();
    int32_t getState();
    //std::string getCurrentServer();

    bool reconnect();
private:
    static void OnWatcher(zhandle_t * zh,int type,int stat,const char * path,void * watcherCtx);
    typedef std::function<void (int type,int stat,const std::string& path)> watcher_callback2;
private:
    //创建zookeeper连接时返回的句柄
    zhandle_t * m_handle;
    //要连接的zookeeper集群
    std::string m_hosts;
    watcher_callback2 m_watcherCb;
    //log_callback m_logCb;
    int32_t m_recvTimeout;

};
}
#endif
