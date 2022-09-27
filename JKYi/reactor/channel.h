#ifndef _JKYI_CHANNEL_H_
#define _JKYI_CHANNEL_H_

#include"JKYi/timestamp.h"

#include<functional>
#include<memory>

namespace JKYi{
namespace net{

class EventLoop;

//每一个channel都代表一个注册了的文件描述符，但是它并不会拥有该文件描述符
class Channel{
public:
    typedef std::shared_ptr<Channel> ptr;
    typedef std::function<void ()> EventCallback;
    typedef std::function<void (Timestamp)> ReadEventCallback;

    Channel(EventLoop* loop,int fd);
    ~Channel();

    void handleEvent(Timestamp receiveTime);

    void setReadCallback(ReadEventCallback cb){ readCallback_ = std::move(cb); }
    void setWriteCallback(EventCallback cb) { writeCallback_ = std::move(cb); }
    void setCloseCallback(EventCallback cb) { closeCallback_ = std::move(cb); } 
    void setErrorCallback(EventCallback cb) { errorCallback_ = std::move(cb); }

    //通过形参中的shared_ptr来对channel进行管理，避免在handevent的过程中该channel被销毁
    void tie(const std::shared_ptr<void>&);

    int getFd()const { return fd_; }
    int getEvents()const { return events_; }
    void setRevents(int rvt) { revents_ = rvt; }

    //判断是否注册有事件
    bool isNoneEvent()const{ return events_ == kNoneEvent; }
    void enableReading() { events_ |= kReadEvent; update();}
    void disableReading() { events_ &= ~kReadEvent; update(); }
    void enableWriting() { events_ |= kWriteEvent; update(); }
    void disableWriting() { events_ &= ~kWriteEvent; update(); }
    void disableAll() { events_ = kNoneEvent; update();}
    bool isReading()const { return events_ & kReadEvent; }
    bool isWriting()const { return events_ & kWriteEvent; }

    int getIndex()const { return index_; }
    void setIndex(int idx){ index_ = idx; }

    std::string reventsToString()const;
    std::string eventsToString()const;

    EventLoop* ownerLoop(){ return loop_; }
    void remove();
private:
    static std::string eventsToString(int fd,int ev);
    void update();
    void handleEventWithGuard(Timestamp timestamp);

    static const int kNoneEvent;
    static const int kReadEvent;
    static const int kWriteEvent;

    EventLoop* loop_;             //属于那一个eventloop
    const int fd_;                // 所代表的文件描述符

    int events_;                  //注册的事件
    int revents_;                 //实际发生的事件
    int index_;                   //poll 作为底层io复用系统调用时使用

    std::weak_ptr<void> tie_;
    bool tied_;
    bool eventHanding_;
    bool addedToLoop_;

    ReadEventCallback readCallback_;  //最底层的读回调
    EventCallback writeCallback_;
    EventCallback closeCallback_;
    EventCallback errorCallback_;
};
}
}

#endif
