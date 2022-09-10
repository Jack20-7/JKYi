#ifndef _JKYI_DB_FOX_THREAD_H_
#define _JKYI_DB_FOX_THREAD_H_

#include<thread>
#include<vector>
#include<list>
#include<map>
#include<event2/bufferevent.h>
#include<event2/buffer.h>
#include<event2/listener.h>

#include"JKYi/singleton.h"
#include"JKYi/mutex.h"

namespace JKYi{

class FoxThread;

//抽象基类
class IFoxThread{
public:
    typedef std::shared_ptr<IFoxThread> ptr;
    typedef std::function<void ()> callback;

    //避免内存泄漏
    virtual ~IFoxThread(){}

    virtual bool dispatch(callback cb) = 0;
    virtual bool dispatch(uint32_t id,callback cb) = 0;
    virtual bool batchDispatch(const std::vector<callback>& cbs) = 0;
    virtual void broadcast(callback cb) = 0;

    virtual void start() = 0;
    virtual void stop() = 0;
    virtual void join() = 0;
    virtual void dump(std::ostream& os) = 0;
    virtual uint64_t getTotal() = 0;
};
//专门用来执行回调函数的线程
class FoxThread : public IFoxThread{
public:
    typedef std::shared_ptr<FoxThread> ptr;
    typedef IFoxThread::callback callback;
    typedef std::function<void (FoxThread*)> init_cb;

    FoxThread(const std::string& name,struct event_base* base = nullptr);
    ~FoxThread();

    static FoxThread* GetThis();
    static const std::string& GetFoxThreadName();
    static void GetAllFoxThreadName(std::map<uint64_t,std::string>& names);

    void setThis();
    void unsetThis();

    bool dispatch(callback cb) override;
    bool dispatch(uint32_t id,callback cb) override;
    bool batchDispatch(const std::vector<callback>&cbs) override;
    void broadcast(callback cb) override;

    void start()override;
    void join() override;
    void stop() override;
    bool isStart()const { return m_start; }

    struct event_base* getBase(){ return m_base; }
    std::thread::id getId()const;

    void* getData(const std::string& name);
    template<class T>
    T* getData(const std::string& name){
        return (T*)getData(name);
    }

    void setData(const std::string& name,void * v);
    void setInitCb(init_cb v) { m_initCb = v; };

    void dump(std::ostream& os);
    uint64_t getTotal() { return m_total; }
private:
    void thread_cb();
    static void read_cb(evutil_socket_t sock,short which,void * args);
private:
    evutil_socket_t m_read;
    evutil_socket_t m_write;
    //reactor
    struct event_base* m_base;
    //事件处理器
    struct event* m_event;
    //真正在底层跑的线程
    std::thread* m_thread;
    //读写锁，用来对回调函数的那个数组进行保护
    JKYi::RWMutex m_mutex;
    std::list<callback> m_callbacks;

    std::string m_name;
    init_cb m_initCb;

    std::map<std::string,void *> m_datas;

    bool m_working;
    bool m_start;
    uint64_t m_total;
};

class FoxThreadPool : public IFoxThread{
public:
    typedef std::shared_ptr<FoxThreadPool> ptr;
    typedef IFoxThread::callback callback;

    FoxThreadPool(uint32_t size,const std::string& name = "",bool advance = false);
    ~FoxThreadPool();

    void start()override;
    void stop()override;
    void join()override;

    //随机选择一个FoxThrea来执行
    bool dispatch(callback cb)override;
    bool batchDispatch(const std::vector<callback>& cbs)override;
    //指定线程来执行
    bool dispatch(uint32_t id,callback cb)override;

    FoxThread* getRandFoxThread();
    void setInitCb(FoxThread::init_cb v){ m_initCb = v; }

    void dump(std::ostream& os)override;
    void broadcast(callback cb)override;

    uint64_t getTotal(){ return m_total; }
private:
    void releaseFoxThread(FoxThread * t);
    void check();
    void wrapCb(std::shared_ptr<FoxThread>,callback cb);
private:
    uint32_t m_size;
    uint32_t m_cur;
    std::string m_name;
    bool m_advance;
    bool m_start;

    RWMutex m_mutex;
    std::list<callback> m_callbacks;

    std::vector<FoxThread*> m_threads;
    std::list<FoxThread*> m_freeFoxThreads;

    FoxThread::init_cb m_initCb;
    uint64_t m_total;
};

class FoxThreadManager{
public:
    typedef IFoxThread::callback callback;

    void dispatch(const std::string& name,callback cb);
    void dispatch(const std::string& name,uint32_t id,callback cb);
    void batchDispatch(const std::string& name,const std::vector<callback>& cbs);
    void broadcast(const std::string& name,callback cb);

    void dumpFoxThreadStatus(std::ostream& os);

    void init();
    void start();
    void stop();

    IFoxThread::ptr get(const std::string& name);
    void add(const std::string& name,IFoxThread::ptr ptr);
private:
    std::map<std::string,IFoxThread::ptr> m_threads;
};

typedef Singleton<FoxThreadManager> FoxThreadMgr;

} 

#endif
