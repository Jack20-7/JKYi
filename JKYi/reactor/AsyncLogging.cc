#include"JKYi/reactor/AsyncLogging.h"
#include"JKYi/timestamp.h"
#include"JKYi/reactor/LogFile.h"

#include<stdio.h>

namespace JKYi{
namespace net{
AsyncLogging::AsyncLogging(const std::string& basename,
                             off_t rollSize,
                             int flushInterval)
    :flushInterval_(flushInterval),
     running_(false),
     basename_(basename),
     rollSize_(rollSize),
     thread_(std::bind(&AsyncLogging::threadFunc,this),"Logging"),
     latch_(1),
     mutex_(),
     cond_(mutex_),
     currentBuffer_(new Buffer),
     nextBuffer_(new Buffer),
     buffers_(){

     currentBuffer_->bzero();
     nextBuffer_->bzero();
     buffers_.reserve(16);
}

void AsyncLogging::append(const char* logline,int len){
    JKYi::Mutex::Lock lock(mutex_);
    if(currentBuffer_->avail() >= len){
        currentBuffer_->append(logline,len);
    }else{
        buffers_.push_back(std::move(currentBuffer_));
        if(nextBuffer_){
            currentBuffer_ = std::move(nextBuffer_);
        }else{
            currentBuffer_.reset(new Buffer);
        }
        currentBuffer_->append(logline,len);
        cond_.notify();
    }
}

void AsyncLogging::threadFunc(){
    printf("log thread start\n");
    assert(running_ == true);
    latch_.countDown();
    LogFile output(basename_,rollSize_,false);
    BufferPtr newBuffer1(new Buffer);
    BufferPtr newBuffer2(new Buffer);
    newBuffer1->bzero();
    newBuffer2->bzero();
    BufferVector buffersToWrite;
    buffersToWrite.reserve(16);

    while(running_){
        printf("into while\n");
        assert(newBuffer1 && newBuffer1->length() == 0);
        assert(newBuffer2 && newBuffer2->length() == 0);
        assert(buffersToWrite.empty());

        {
            JKYi::Mutex::Lock lock(mutex_);
            if(buffers_.empty()){
                cond_.waitForSeconds(flushInterval_);
            }
            buffers_.push_back(std::move(currentBuffer_));
            currentBuffer_ = std::move(newBuffer1);
            buffersToWrite.swap(buffers_);
            if(!nextBuffer_){
                nextBuffer_ = std::move(newBuffer2);
            }
        }
        assert(!buffersToWrite.empty());

        //如果写日志的速度过于的快的话，那么应该是出现的问题
        if(buffersToWrite.size() > 25){
            char buf[256];
            snprintf(buf,sizeof buf,"Dropped log messages at %s,%zd large buffers\n",
                          Timestamp::now().toFormattedString().c_str(),
                          buffersToWrite.size() - 2);
            fputs(buf,stderr);
            output.append(buf,static_cast<int>(strlen(buf)));  //错误信息写入文件
            buffersToWrite.erase(buffersToWrite.begin() + 2,buffersToWrite.end());
        }
        //依次将Buffer中的数据写入到日志文件里面去
        printf("buffersToWirte size = %d\n",buffersToWrite.size());
        for(const auto& buffer : buffersToWrite){
            output.append(buffer->data(),buffer->length());
        }
        if(buffersToWrite.size() > 2){
            buffersToWrite.resize(2);
        }
        if(!newBuffer1){
            assert(!buffersToWrite.empty());
            newBuffer1 = std::move(buffersToWrite.back());
            buffersToWrite.pop_back();
            newBuffer1->reset();
        }
        if(!newBuffer2){
            assert(!buffersToWrite.empty());
            newBuffer2 = std::move(buffersToWrite.back());
            buffersToWrite.pop_back();
            newBuffer2->reset();
        }
        buffersToWrite.clear();
        output.flush();
    }
    output.flush();
}

}
}
