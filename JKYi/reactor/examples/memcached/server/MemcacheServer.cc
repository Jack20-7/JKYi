#include"JKYi/reactor/examples/memcached/server/MemcacheServer.h"

#include"JKYi/atomic.h"
#include"JKYi/reactor/Logging.h"
#include"JKYi/reactor/EventLoop.h"

using namespace JKYi;
using namespace JKYi::net;

JKYi::AtomicInt64 g_cas;

MemcacheServer::Options::Options(){
    memset(this,0,sizeof(*this));
}

//这个的作用我不是很理解
struct MemcacheServer::Stats{
};

MemcacheServer::MemcacheServer(EventLoop* loop,const Options& options)
    :loop_(loop),
     options_(options),
     startTime_(::time(NULL) - 1),
     server_(loop,Address::LookupAnyIPAddress("0.0.0.0",std::to_string(options.tcpport)),
             "JKYi-memcached"),
     stats_(new Stats){

     server_.setConnectionCallback(
             std::bind(&MemcacheServer::onConnection,this,_1));
}

MemcacheServer::~MemcacheServer() = default;

void MemcacheServer::start(){
    server_.start();
}

void MemcacheServer::stop(){
    loop_->runAfter(3.0,std::bind(&EventLoop::quit,loop_));
}

bool MemcacheServer::storeItem(const ItemPtr& item,
                               const Item::UpdatePolicy policy,
                               bool *exists){
    assert(item->neededBytes() == 0);
    Mutex& mutex = shards_[item->hash() % kShards].mutex;
    ItemMap& items = shards_[item->hash() % kShards].items;
    Mutex::Lock lock(mutex);
    ItemMap::const_iterator it = items.find(item);
    *exists = (it != items.end());
    if(policy == Item::kSet){
        item->setCas(g_cas.incrementAndGet());
        if(*exists){
            items.erase(it);
        }
        items.insert(item);
    }else if(policy == Item::kAdd){
        if(*exists){
            return false;
        }else{
            item->setCas(g_cas.incrementAndGet());
            items.insert(item);
        }
    }else if(policy == Item::kReplace){
        if(*exists){
             item->setCas(g_cas.incrementAndGet());
             items.erase(it);
             items.insert(item);
        }else{
            return false;
        }
    }else if(policy == Item::kAppend || policy == Item::kPrepend){
        //append是用来在已存在的value后面追加数据
        //prepend是用来在已存在的value前面追加数据
        if(*exists){
            const ConstItemPtr& oldItem = *it;
            int newLen = static_cast<int>(item->valueLength() + oldItem->valueLength()-2);
            ItemPtr newItem(Item::makeItem(item->key(),
                                           oldItem->flags(),
                                           oldItem->rel_exptime(),
                                           newLen,
                                           g_cas.incrementAndGet()));
            if(policy == Item::kAppend){
                newItem->append(oldItem->value(),oldItem->valueLength() - 2);
                newItem->append(item->value(),item->valueLength());
            }else{
                newItem->append(item->value(),item->valueLength() - 2);
                newItem->append(oldItem->value(),oldItem->valueLength());
            }
            assert(newItem->neededBytes() == 0);
            assert(newItem->endsWithCRLF());
            items.erase(it);
            items.insert(newItem);
        }else{
            return false;
        }
    }else if(policy == Item::kCas){
        //只有cas相同才能够被修改，相当于是同一个版本的
        if(*exists && (*it)->cas() == item->cas()){
            item->setCas(g_cas.incrementAndGet());
            items.erase(it);
            items.insert(item);
        }else{
            return false;
        }
    }else{
        assert(false);
    }
    return true;
}

ConstItemPtr MemcacheServer::getItem(const ConstItemPtr& key)const{
    Mutex& mutex = shards_[key->hash() % kShards].mutex;
    const ItemMap& items = shards_[key->hash() % kShards].items;
    Mutex::Lock lock(mutex);
    ItemMap::const_iterator it = items.find(key);
    return it != items.end() ? *it : ConstItemPtr();
}

bool MemcacheServer::deleteItem(const ConstItemPtr& key){
    Mutex& mutex = shards_[key->hash() % kShards].mutex;
    ItemMap& items = shards_[key->hash() % kShards].items;
    Mutex::Lock lock(mutex);
    return items.erase(key) == 1;
}

void MemcacheServer::onConnection(const TcpConnectionPtr& conn){
    if(conn->connected()){
        SessionPtr session(new Session(this,conn));
        Mutex::Lock lock(mutex_);
        assert(sessions_.find(conn->name()) == sessions_.end());
        sessions_[conn->name()] = session;
    }else{
        Mutex::Lock lock(mutex_);
        assert(sessions_.find(conn->name()) != sessions_.end());
        sessions_.erase(conn->name());
    }
    return ;
}



