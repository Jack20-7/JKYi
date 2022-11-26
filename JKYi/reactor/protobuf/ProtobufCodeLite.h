#ifndef _JKYI_PROTOBUF_PROTOBUFCODELITE_H_
#define _JKYI_PROTOBUF_PROTOBUFCODELITE_H_

#include"boost/noncopyable.hpp"
#include"JKYi/reactor/StringPiece.h"
#include"JKYi/timestamp.h"
#include"JKYi/reactor/Callbacks.h"
#include"JKYi/reactor/TcpConnection.h"

#include<memory>
#include<type_traits>
#include<string>


namespace google{
namespace protobuf{
class Message;
}
}

namespace JKYi{
namespace net{

typedef std::shared_ptr<google::protobuf::Message> MessagePtr;

//消息的基本格式为
//
// Field     Length  Content
//
// size      4-byte  M+N+4
// tag       M-byte  could be "RPC0", etc.
// payload   N-byte
// checksum  4-byte  adler32 of tag+payload

class ProtobufCodecLite : boost::noncopyable{
public:
    const static int kHeaderLen = sizeof(int32_t);
    const static int kChecksumLen = sizeof(int32_t);
    const static int kMaxMessageLen = 64 * 1024 * 1024;

    enum ErrorCode{
        kNoError  = 0,
        kInvalidLength = 1,
        kCheckSumError = 2,
        kInvalidNameLen = 3,
        kUnknowMessageType = 4,
        kParseError = 5
    };
    typedef std::function<bool (const TcpConnection::ptr& ,
                                 StringPiece,
                                 Timestamp)> RawMessageCallback;
    typedef std::function<void (const TcpConnection::ptr&,
                                const MessagePtr&,
                                Timestamp)> ProtobufMessageCallback; 
    typedef std::function<void (const TcpConnection::ptr&,
                                Buffer*,
                                Timestamp,
                                ErrorCode)> ErrorCallback;

    ProtobufCodecLite(const ::google::protobuf::Message* prototype,
                       StringPiece tagArg,
                       const ProtobufMessageCallback& messageCb,
                       const RawMessageCallback& rawCb = RawMessageCallback(),
                       const ErrorCallback& errorCb = defaultErrorCallback)
        :prototype_(prototype),
         tag_(tagArg.as_string()),
         messageCallback_(messageCb),
         rawCb_(rawCb),
         errorCallback_(errorCb),
         kMinMessageLen(tagArg.size() + kChecksumLen){}

    virtual ~ProtobufCodecLite() = default;
    const std::string& tag()const { return tag_; }

    void send(const TcpConnection::ptr& conn,const ::google::protobuf::Message& message);
    void onMessage(const TcpConnection::ptr& conn,Buffer* buf,Timestamp receiveTime);  //注册到TcpConnection的函数

    virtual bool parseFromBuffer(StringPiece buf,google::protobuf::Message* message);  //将buffer中的数据转化为对于类型的message返回
    virtual int serializeToBuffer(const ::google::protobuf::Message& message,
                                                                        Buffer* buf);
    static const std::string& errorCodeToString(ErrorCode errorCode);

    ErrorCode parse(const char * buf,int len,::google::protobuf::Message* message);
    void fillEmptyBuffer(Buffer* buf,const ::google::protobuf::Message& message);

    static int32_t checksum(const void * buf,int len);
    static bool validateChecksum(const char* buf,int len);
    static int32_t asInt32(const void * buf);
    static void defaultErrorCallback(const TcpConnection::ptr& ,
                                     Buffer* ,
                                     Timestamp,
                                     ErrorCode);
private:
    const ::google::protobuf::Message* prototype_;
    const std::string tag_;

    ProtobufMessageCallback messageCallback_;  //该函数就是dispatcher指定的绑定的函数
    RawMessageCallback rawCb_;
    ErrorCallback errorCallback_;

    const int kMinMessageLen;
};

//代替了dispatcher的作用，该模板类的一个具体实例对应一个具体Message类型
template<typename MSG,const char * TAG,typename CODEC = ProtobufCodecLite>
class ProtobufCodecLiteT{
public:
    typedef std::shared_ptr<MSG> ConcreteMessagePtr;
    typedef std::function<void (const TcpConnection::ptr&,
                                const ConcreteMessagePtr&,
                                Timestamp)> ProtobufMessageCallback;
    typedef ProtobufCodecLite::RawMessageCallback RawMessageCallback;
    typedef ProtobufCodecLite::ErrorCallback ErrorCallback;

    explicit ProtobufCodecLiteT(const ProtobufMessageCallback& messageCb,
                                const RawMessageCallback& rawCb = RawMessageCallback(),
                                const ErrorCallback& errorCb = ProtobufCodecLite::defaultErrorCallback)
        :messageCallback_(messageCb),
         codec_(&MSG::default_instance(),
                 TAG,
                 std::bind(&ProtobufCodecLiteT::onRpcMessage,this,_1,_2,_3),
                 rawCb,
                 errorCb){}
    const std::string& tag()const { return codec_.tag(); }

    void send(const TcpConnection::ptr& conn, const MSG& message){
        codec_.send(conn,message);
    }
    void onMessage(const TcpConnection::ptr& conn,Buffer* buf,Timestamp receiveTime){
        codec_.onMessage(conn,buf,receiveTime);
    }
    void onRpcMessage(const TcpConnection::ptr& conn,const MessagePtr& message,Timestamp receiveTime){
        messageCallback_(conn,down_pointer_cast<MSG>(message),receiveTime);
    }
    void fillEmptyBuffer(Buffer* buf,const MSG& message){
        codec_.fillEmptyBuffer(buf,message);
    }


private:
    ProtobufMessageCallback messageCallback_;
    CODEC codec_;
};

} //namespace JKYi
} //namespace net

#endif
