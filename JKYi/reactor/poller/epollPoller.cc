#include"JKYi/reactor/poller/epollPoller.h"
#include"JKYi/log.h"
#include"JKYi/reactor/channel.h"
#include"JKYi/macro.h"

#include<assert.h>
#include<errno.h>
#include<poll.h>
#include<sys/epoll.h>
#include<unistd.h>

namespace JKYi{
namespace net{

static Logger::ptr g_logger = JKYI_LOG_NAME("system");

static_assert(EPOLLIN == POLLIN,        "epoll uses same flag values as poll");
static_assert(EPOLLPRI == POLLPRI,      "epoll uses same flag values as poll");
static_assert(EPOLLOUT == POLLOUT,      "epoll uses same flag values as poll");
static_assert(EPOLLRDHUP == POLLRDHUP,  "epoll uses same flag values as poll");
static_assert(EPOLLERR == POLLERR,      "epoll uses same flag values as poll");
static_assert(EPOLLHUP == POLLHUP,      "epoll uses same flag values as poll");

namespace {
    //用来表示channel的状态
    const int kNew  = -1;     //未添加
    const int kAdded = 1;     //已添加
    const int kDeleted = 2;   //map中存在该channel，但是对应的fd已经epoll_ctl_del
}

EpollPoller::EpollPoller(EventLoop* loop)
    :Poller(loop),
     epollfd_(::epoll_create1(EPOLL_CLOEXEC)),
     events_(kInitEventListSize){
    if(epollfd_ < 0){
        JKYI_LOG_ERROR(g_logger) << " epoll_creata1 error";
    }
}

EpollPoller::~EpollPoller(){
    ::close(epollfd_);
}


Timestamp EpollPoller::poll(int timeoutMs,ChannelList* activeChannels){
    JKYI_LOG_DEBUG(g_logger) << " fd total count " << channels_.size();

    int numEvents = ::epoll_wait(epollfd_,&*events_.begin(),
                                 static_cast<int>(events_.size()),timeoutMs);
    int savedErrno = errno;
    Timestamp now(Timestamp::now());
    if(numEvents > 0){
        JKYI_LOG_DEBUG(g_logger) << numEvents << " events happened";
        fillActiveChannels(numEvents,activeChannels);
        if(static_cast<size_t>(numEvents) == events_.size()){
            events_.resize(2 * events_.size());
        }
    }else if(numEvents == 0){
        JKYI_LOG_DEBUG(g_logger) << " no events happended";
    }else{
        if(savedErrno != EINTR){
            errno = savedErrno;
            JKYI_LOG_ERROR(g_logger) << " epoll_wait error";
        }
    }
    return now;
}
void EpollPoller::fillActiveChannels(int numEvents,ChannelList* activeChannels)const{
    JKYI_ASSERT(static_cast<size_t>(numEvents) <= events_.size());
    for(int i = 0;i < numEvents;++i){
        Channel* channel = static_cast<Channel*>(events_[i].data.ptr);
#ifdef NDEBUG
        int fd = channel->getFd();
        ChannelMap::const_iterator it = channels_.find(fd);
        JKYI_ASSERT(it != channels_.end());
        JKYI_ASSERT(it->second == channel);
#endif
      channel->setRevents(events_[i].events);
      activeChannels->push_back(channel);
    }
}

void EpollPoller::updateChannel(Channel* channel){
    Poller::assertInLoopThread();
    const int index = channel->getIndex();
    JKYI_LOG_DEBUG(g_logger) << " updateChannel fd = " << channel->getFd() 
                             << " events = " << channel->getEvents()
                             << " index = " << index;
    if(index == kNew || index == kDeleted){
        int fd = channel->getFd();
        if(index == kNew){
            JKYI_ASSERT(channels_.find(fd) == channels_.end());
            channels_[fd] = channel;
        }else{
            //kDeleted
            JKYI_ASSERT(channels_.find(fd) != channels_.end());
            JKYI_ASSERT(channels_[fd] == channel);
        }
        channel->setIndex(kAdded);
        update(EPOLL_CTL_ADD,channel);
    }else{
        //kAdded
        int fd = channel->getFd();

        JKYI_ASSERT(channels_.find(fd) != channels_.end());
        JKYI_ASSERT(channels_[fd] == channel);
        JKYI_ASSERT(index == kAdded);

        if(channel->isNoneEvent()){
            update(EPOLL_CTL_DEL,channel);
            channel->setIndex(kDeleted);
        }else{
            update(EPOLL_CTL_MOD,channel);
        }
    }
}
void EpollPoller::removeChannel(Channel* channel){
    Poller::assertInLoopThread();
    int fd = channel->getFd();
    JKYI_LOG_DEBUG(g_logger) << " removeChannel fd = " << fd;

    JKYI_ASSERT(channels_.find(fd) != channels_.end());
    JKYI_ASSERT(channels_[fd] == channel);
    JKYI_ASSERT(channel->isNoneEvent());

    int index = channel->getIndex();
    JKYI_ASSERT(index == kAdded || index == kDeleted);

    channels_.erase(fd);
    if(index == kAdded){
        update(EPOLL_CTL_DEL,channel);
    }
    channel->setIndex(kNew);
}
void EpollPoller::update(int operation,Channel* channel){
    struct epoll_event event;
    memset(&event,0,sizeof(event));
    event.events = channel->getEvents();
    event.data.ptr = channel;
    int fd = channel->getFd();
    JKYI_LOG_DEBUG(g_logger) << " epoll_ctl op = " << operationToString(operation)
                             << " fd = " << fd << " events = (" 
                             << channel->eventsToString() << " )";
    if(::epoll_ctl(epollfd_,operation,fd,&event) < 0){
        JKYI_LOG_DEBUG(g_logger) << " epoll_ctl op = " 
                                 << operationToString(operation) 
                                 << " fd = "<< fd;
    }
    return ;
}
const char * EpollPoller::operationToString(int op){
    switch(op){
        case EPOLL_CTL_ADD:
            return "ADD";
        case EPOLL_CTL_DEL:
            return "DEL";
        case EPOLL_CTL_MOD:
            return "MOD";
        default:
            return "Unknow operation";
    }
}


}
}
