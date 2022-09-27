#ifndef _JKYI_POLLER_H_
#define _JKYI_POLLER_H_

#include<map>
#include<vector>

#include"JKYi/timestamp.h"
#include"JKYi/reactor/EventLoop.h"

namespace JKYi{
namespace net{

class Channel;

//IO事件分发器的抽象类
class Poller:public Noncopyable{
public:
    typedef std::vector<Channel*> ChannelList;

    Poller(EventLoop* loop);
    virtual ~Poller();

    virtual Timestamp poll(int timeoutMs,ChannelList* activeChannels) = 0;
    virtual void updateChannel(Channel* channel) = 0;
    virtual void removeChannel(Channel* channel) = 0;
    virtual bool hasChannel(Channel* channel) const;

    static Poller* newDefaultPoller(EventLoop* loop);

    void assertInLoopThread()const{ ownerLoop_->assertInLoopThread(); }

protected:
    typedef std::map<int,Channel*> ChannelMap;
    ChannelMap channels_;
private:
    EventLoop* ownerLoop_;
};
}
}
#endif
