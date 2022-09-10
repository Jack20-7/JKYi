#ifndef _JKYI_BLOCKING_QUEUE_H_
#define _JKYI_BLOCKING_QUEUE_H_

#include"JKYi/mutex.h"

#include<list>
#include<memory>

namespace JKYi{
namespace ds{

//自定实现阻塞队列
template<class T>
class BlockingQueue{
public:
    typedef std::shared_ptr<BlockingQueue> ptr;
    typedef std::shared_ptr<T> data_type;
    typedef JKYi::SpinLock MutexType;

    size_t push(const data_type& data){
        MutexType::Lock lock(m_mutex);
        m_datas.push_back(data);
        size_t size = m_datas.size();
        lock.unlock();
        m_sem.nofity();
        return size;
    }

    data_type pop(){
        m_sem.wait();
        MutexType::Lock lock(m_mutex);
        auto v = m_datas.front();
        m_datas.pop_front();
        return v;
    }

    size_t size(){
        MutexType::Lock lock(m_mutex);
        return m_datas.size();
    }
    bool empty(){
        MutexType::Lock lock(m_mutex);
        return m_datas.empty();
    }
private:
    //协程信号量
    JKYi::FiberSemaphore m_sem;

    //用于对下面的list进行保护
    MutexType m_mutex;
    std::list<data_type> m_datas;
};

}
}

#endif
