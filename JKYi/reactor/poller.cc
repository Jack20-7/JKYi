#include"JKYi/reactor/poller.h"
#include"JKYi/reactor/channel.h"

namespace JKYi{
namespace net{

Poller::Poller(EventLoop* loop)
    :ownerLoop_(loop){
}

Poller::~Poller() = default;

bool Poller::hasChannel(Channel* channel)const{
    assertInLoopThread();
    ChannelMap::const_iterator it = channels_.find(channel->getFd());
    return it != channels_.end() && it->second == channel;
}

}
}
