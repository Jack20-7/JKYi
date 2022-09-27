#ifndef _JKYI_EPOLLPOLLER_H_
#define _JKYI_EPOLLPOLLER_H_

#include"JKYi/reactor/poller.h"

#include<vector>

struct epoll_event;

namespace JKYi{
namespace net{

//基于epoll实现的io事件分发器

class EpollPoller:public Poller{
public:
    EpollPoller(EventLoop* loop);
    ~EpollPoller()override;

    Timestamp poll(int timeoutMs,ChannelList* activeChannels)override;
    void updateChannel(Channel* channel)override;
    void removeChannel(Channel* channel)override;
private:
    static const int kInitEventListSize = 16;
    static const char * operationToString(int op);

    void fillActiveChannels(int numEvents,ChannelList* activeChannels) const;
    void update(int operation,Channel* channel);

    typedef std::vector<struct epoll_event> EventList;

    int epollfd_;
    EventList events_;
};
}
}

#endif
