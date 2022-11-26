#ifndef _JKYI_NET_EXAMPLES_H_
#define _JKYI_NET_EXAMPLES_H_

#include"JKYi/reactor/Logging.h"
#include"JKYi/reactor/Buffer.h"
#include"JKYi/endian.h"
#include"JKYi/reactor/TcpConnection.h"
#include"JKYi/noncopyable.h"

#include<string>

class LengthHeaderCodec : public JKYi::Noncopyable{
public:
    typedef std::function<void (const JKYi::net::TcpConnectionPtr& conn,
                                 const std::string& message,
                                 JKYi::net::Timestamp receiveTime)> StringMessageCallback;

    explicit LengthHeaderCodec(const StringMessageCallback& cb)
        :messageCallback_(cb){
    }

    void onMessage(const JKYi::net::TcpConnectionPtr& conn,
                    JKYi::net::Buffer* buf,
                    JKYi::net::Timestamp receiveTime){
        while(buf->readableBytes() >= kHeaderLen){
            const void* data = buf->peek();
            uint32_t be32 = *static_cast<const uint32_t*>(data);
            const int32_t len = JKYi::byteswap(be32);
            if(len > 65535 || len < 0){
                LOG_ERROR << "Invalid length: " <<  len;
                conn->shutdown();
                break;
            }else if(buf->readableBytes() >= len + kHeaderLen){
                //如果收到的一条完整的消息
                buf->retrieve(kHeaderLen);
                std::string message(buf->peek(),len);
                messageCallback_(conn,message,receiveTime);
                buf->retrieve(len);
            }else{
                break;
            }
        }
        return ;
    }
    void send(JKYi::net::TcpConnection* conn,
                 const JKYi::net::StringPiece& message){
        JKYi::net::Buffer buf;
        buf.append(message.data(),message.size());
        uint32_t len = static_cast<int32_t>(message.size());
        uint32_t be32 = JKYi::byteswap(len);
        buf.prepend(&be32,sizeof(be32));
        conn->send(&buf);
    }
private:
        StringMessageCallback messageCallback_;
        const static size_t kHeaderLen = sizeof(int32_t);
};

#endif
