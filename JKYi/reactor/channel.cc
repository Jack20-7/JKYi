#include"JKYi/reactor/channel.h"
#include"JKYi/log.h"
#include"JKYi/reactor/EventLoop.h"
#include"JKYi/macro.h"

#include<sstream>
#include<poll.h>

namespace JKYi{
namespace net{
//static成员变量类内声明，类外进行初始化
//如果有const修饰的话，那么就可以直接在类内进行初始化

static Logger::ptr g_logger = JKYI_LOG_NAME("system");

const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = POLLIN | POLLPRI;
const int Channel::kWriteEvent = POLLOUT;

Channel::Channel(EventLoop* loop,int fd)
    :loop_(loop),
     fd_(fd),
     events_(0),
     revents_(0),
     index_(-1),
     tied_(false),
     eventHanding_(false),
     addedToLoop_(false){
}

Channel::~Channel(){
    JKYI_ASSERT(!eventHanding_);
    JKYI_ASSERT(!addedToLoop_);
    if(loop_->isInLoopThread()){
        JKYI_ASSERT(!loop_->hasChannel(this));
    }
}
void Channel::tie(const std::shared_ptr<void>& obj){
    tie_ = obj;
    tied_ = true;
}
void Channel::update(){
    addedToLoop_ = true;
    loop_->updateChannel(this);
}

void Channel::remove(){
    JKYI_ASSERT(isNoneEvent());
    addedToLoop_ = false;
    loop_->removeChannel(this);
}

void Channel::handleEvent(Timestamp receiveTime){
    std::shared_ptr<void> guard;
    if(tied_){
        guard = tie_.lock();
        if(guard){
            handleEventWithGuard(receiveTime);
        }
    }else{
        handleEventWithGuard(receiveTime);
    }
    return ;
}

void Channel::handleEventWithGuard(Timestamp receiveTime){
    eventHanding_ = true;
    JKYI_LOG_DEBUG(g_logger) << reventsToString(); 

    if((revents_ & POLLHUP) && !(revents_ & POLLIN)){
        JKYI_LOG_WARN(g_logger)<< "fd = " << fd_ << " Channel::handleEvent() POLLHUP";
        if(closeCallback_){
            closeCallback_();
        }
    }

    if(revents_ & POLLNVAL){
        JKYI_LOG_WARN(g_logger) << " fd =  " << fd_ << " Channel::handleEvent pollnval";
    }

    if(revents_ & (POLLERR | POLLNVAL)){
        if(errorCallback_){
            errorCallback_();
        }
    }

    if(revents_ & (POLLIN | POLLPRI | POLLRDHUP)){
        if(readCallback_){
            readCallback_(receiveTime);
        }
    }

    if(revents_ & POLLOUT){
        if(writeCallback_){
            writeCallback_();
        }
    }
    eventHanding_ = false;
}

std::string Channel::reventsToString()const{
    return eventsToString(fd_,revents_);
}
std::string Channel::eventsToString()const{
    return eventsToString(fd_,events_);
}

std::string Channel::eventsToString(int fd,int ev){
    std::stringstream ss;
    ss << fd << " : ";
#define XX(ev,val)\
    if(ev & val){\
     ss << #val;\
    }
    XX(ev,POLLIN);
    XX(ev,POLLPRI);
    XX(ev,POLLOUT);
    XX(ev,POLLHUP);
    XX(ev,POLLRDHUP);
    XX(ev,POLLERR);
    XX(ev,POLLNVAL);
#undef XX
   return ss.str();
}
}
}
