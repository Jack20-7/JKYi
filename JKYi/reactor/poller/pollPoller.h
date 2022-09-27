#ifndef _JKYI_POLLPOLLER_H_
#define _JKYI_POLLPOLLER_H_

#include"JKYi/reactor/poller.h"

#include<vector>

struct pollfd;

namespace JKYi{
namespace net{

//基于poll实现的io事件分发器
class PollPoller:public Poller{
public:
    PollPoller(EventLoop* loop);
    ~PollPoller()override;

    Timestamp poll(int timeoutMs,ChannelList* activeChannels)override;
    void updateChannel(Channel* channel)override;
    void removeChannel(Channel* channel)override;
private:
   void fillActiveChannels(int numEvents,ChannelList* activeChannels);
   typedef std::vector<struct pollfd> PollFdList;

   PollFdList pollfds_;
};

}
}


#endif
