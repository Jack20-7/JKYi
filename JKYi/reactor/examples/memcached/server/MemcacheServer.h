#ifndef _JKYI_NET_MEMCACHE_SERVER_H_
#define _JKYI_NET_MEMCACHE_SERVER_H_

#include"JKYi/reactor/examples/memcached/server/Item.h"
#include"JKYi/mutex.h"
#include"JKYi/reactor/TcpServer.h"
#include"JKYi/reactor/examples/memcached/server/Session.h"

#include<array>
#include<string>
#include<unordered_map>
#include<unordered_set>

class MemcacheServer : JKYi::Noncopyable{
public:
    struct Options{
        Options();

        uint16_t tcpport;
        uint16_t udpport;
        uint16_t gperfport;
        int threads;
    };

    MemcacheServer(JKYi::net::EventLoop* loop,const Options& options);
    ~MemcacheServer();

    void setThreadNum(int threads){
        server_.setThreadNum(threads);
    }
    void start();
    void stop();

    time_t startTime()const{
        return startTime_;
    }

    //实现的三大操作
    bool storeItem(const ItemPtr& item,Item::UpdatePolicy policy,bool* exists);
    ConstItemPtr getItem(const ConstItemPtr& key)const;
    bool deleteItem(const ConstItemPtr& key);
private:
    void onConnection(const JKYi::net::TcpConnectionPtr& conn);

    struct Stats;      //这个东西不知道有什么作用

    JKYi::net::EventLoop* loop_;
    Options options_;
    const time_t startTime_;

    JKYi::Mutex mutex_;
    std::unordered_map<std::string,SessionPtr> sessions_;

    //根据要存储的Item自己定义hash函数
    struct Hash{
        size_t operator() (const ConstItemPtr& x)const{
            return x->hash();
        }
    };
    //自定义排序规则
    struct Equal{
        size_t operator() (const ConstItemPtr& x,const ConstItemPtr& y)const{
            return x->key() == y->key();
        }
    };

    typedef std::unordered_set<ConstItemPtr,Hash,Equal> ItemMap;

    struct MapWithLock{
        ItemMap items;
        mutable JKYi::Mutex mutex;
    };
    const static int kShards = 4096;
    //哈希数组
    std::array<MapWithLock,kShards> shards_;

    JKYi::net::TcpServer server_;
    std::unique_ptr<Stats> stats_;
};

#endif
