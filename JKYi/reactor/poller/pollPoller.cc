#include"JKYi/reactor/poller/pollPoller.h"
#include"JKYi/log.h"
#include"JKYi/macro.h"
#include"JKYi/reactor/channel.h"

#include<errno.h>
#include<poll.h>

namespace JKYi{
namespace net{

static Logger::ptr g_logger = JKYI_LOG_NAME("system");

PollPoller::PollPoller(EventLoop* loop)
    :Poller(loop){
}

PollPoller::~PollPoller() = default;

Timestamp PollPoller::poll(int timeoutMs,ChannelList* activeChannels){
    int numEvents = ::poll(&*pollfds_.begin(),pollfds_.size(),timeoutMs);
    int savedErrno = errno;
    Timestamp now(Timestamp::now());
    if(numEvents > 0){
        JKYI_LOG_DEBUG(g_logger) << numEvents << " events happened";
        fillActiveChannels(numEvents,activeChannels);
    }else if(numEvents == 0){
        JKYI_LOG_DEBUG(g_logger) << " no events happened";
    }else{
        if(savedErrno != EINTR){
            errno = savedErrno;
            JKYI_LOG_ERROR(g_logger) << " poll error ";
        }
    }
    return now;
}

void PollPoller::updateChannel(Channel * channel){
    Poller::assertInLoopThread();

    JKYI_LOG_DEBUG(g_logger) << " updateChannel fd = " << channel->getFd()
                             << " events = " << channel->getEvents();

    //判断该channel对应的文件描述符是否注册过
    if(channel->getIndex() < 0){
        //如果不存在的话
        JKYI_ASSERT(channels_.find(channel->getFd()) == channels_.end());
        struct pollfd pfd;
        pfd.fd = channel->getFd();
        pfd.events = static_cast<short>(channel->getEvents());
        pfd.revents = 0;
        pollfds_.push_back(pfd);
        int index = static_cast<int>(pollfds_.size()) - 1;
        channel->setIndex(index);
        channels_[pfd.fd] = channel;
    }else{
        //如果存在数组中的话
        JKYI_ASSERT(channels_.find(channel->getFd()) != channels_.end());
        JKYI_ASSERT(channels_[channel->getFd()] == channel);

        int index = channel->getIndex();
        JKYI_ASSERT(index >= 0 && index < static_cast<int>(pollfds_.size()));
        struct pollfd& pfd = pollfds_[index];
        JKYI_ASSERT(pfd.fd == channel->getFd() || pfd.fd == - channel->getFd() - 1);

        pfd.fd = channel->getFd();
        pfd.events = static_cast<short>(channel->getEvents());
        pfd.revents = 0;
        if(channel->isNoneEvent()){
          //忽略该fd
          pfd.fd = - channel->getFd() - 1;
        }
    }
}

void PollPoller::removeChannel(Channel * channel){
    Poller::assertInLoopThread();
    JKYI_LOG_DEBUG(g_logger) << " removeChannel fd = " << channel->getFd();

    JKYI_ASSERT(channels_.find(channel->getFd()) != channels_.end());
    JKYI_ASSERT(channels_[channel->getFd()] == channel);
    JKYI_ASSERT(channel->isNoneEvent());

    int index = channel->getIndex();
    JKYI_ASSERT(index >= 0 && index < static_cast<int>(pollfds_.size()));

    const struct pollfd& pfd = pollfds_[index];
    JKYI_ASSERT(pfd.fd == (- channel->getFd() - 1) 
                   && pfd.events == channel->getEvents()); 
    //首先从map容器中进行删除
    channels_.erase(channel->getFd());
    //然后需要从pollfd数组中进行删除，在删除时可以判断是否就在尾部
    if(static_cast<size_t>(index) == pollfds_.size() - 1){
        //如果就是尾部元素的话 
        pollfds_.pop_back();
    }else{
        ///如果在中间的话，那么就需要先将该pollfd交换到尾部，在pop_back 
        int channelEnd = pollfds_.back().fd;
        iter_swap(pollfds_.begin() + index,pollfds_.end() - 1);
        if(channelEnd < 0){
            channelEnd = - channelEnd - 1;
        }
        channels_[channelEnd]->setIndex(index);
        pollfds_.pop_back();
    }
}
void PollPoller::fillActiveChannels(int numEvents,ChannelList* activeChannels){
    for(PollFdList::const_iterator it = pollfds_.begin();
                                      it != pollfds_.end() && numEvents > 0;++it){
        if(it->revents > 0){
            --numEvents;
            ChannelMap::const_iterator ch = channels_.find(it->fd);
            JKYI_ASSERT(ch != channels_.end());

            Channel* channel = ch->second;
            JKYI_ASSERT(channel->getFd() == it->fd);

            channel->setRevents(it->revents);
            activeChannels->push_back(channel);
        }
    }
    return ;
}

}
}
