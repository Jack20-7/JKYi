#include"JKYi/reactor/protobuf/ProtobufCodeLite.h"
#include"JKYi/log.h"
#include"JKYi/endian.h"
#include"JKYi/reactor/TcpConnection.h"
#include"JKYi/reactor/protorpc/google-inl.h"
#include"JKYi/macro.h"
#include"google/protobuf/message.h"
#include"zlib.h"

using namespace JKYi;
using namespace JKYi::net;

static Logger::ptr g_logger = JKYI_LOG_NAME("system");
namespace {
    int ProtobufVersionCheck(){
        GOOGLE_PROTOBUF_VERIFY_VERSION;
        return 0;
    }
    int __attribute__((unused))dummy = ProtobufVersionCheck();
}

void ProtobufCodecLite::fillEmptyBuffer(JKYi::net::Buffer* buf,
                              const ::google::protobuf::Message& message){
    JKYI_ASSERT(buf->readableBytes() == 0);
    buf->append(tag_);  //将tag写入buffer
    int byte_size = serializeToBuffer(message,buf);//将message序列化到buffer里面去
    //通过tag的值 和 message的值 计算出一个校验和
    int32_t checkSum = checksum(buf->peek(),static_cast<int>(buf->readableBytes()));
    buf->appendInt32(checkSum);
    JKYI_ASSERT(buf->readableBytes() == tag_.size() + byte_size + kChecksumLen);

    int32_t len = JKYi::toNetEndian(static_cast<int32_t>(buf->readableBytes()));
    buf->prepend(&len,sizeof(len));
}

void ProtobufCodecLite::send(const TcpConnection::ptr& conn,
                              const ::google::protobuf::Message& message){
    JKYi::net::Buffer buf;
    fillEmptyBuffer(&buf,message);
    conn->send(&buf);
}
//其实该函数就是到Tcpconnection的回调函数
void ProtobufCodecLite::onMessage(const TcpConnection::ptr& conn,
                                  Buffer* buf,
                                  Timestamp receiveTime){
    while(buf->readableBytes() >= static_cast<uint32_t>(kMinMessageLen + kHeaderLen)){
        //取到len字段
        const int32_t len = buf->peekInt32();
        if(len > kMaxMessageLen || len < kMinMessageLen){
            //如果len字段的大小不合理
            errorCallback_(conn,buf,receiveTime,kInvalidLength);
            break;
        }else if(buf->readableBytes() >= static_cast<size_t>(kHeaderLen + len)){
            //长度合理
            if(rawCb_ && !rawCb_(conn,StringPiece(buf->peek(),kHeaderLen + len),
                                receiveTime)){
                buf->retrieve(kHeaderLen + len);
                continue;
            }
            MessagePtr message(prototype_->New());
            ErrorCode errorcode = parse(buf->peek() + kHeaderLen,len,message.get());
            if(errorcode == kNoError){
                messageCallback_(conn,message,receiveTime); 
                buf->retrieve(kHeaderLen + len);
            }else{
                errorCallback_(conn,buf,receiveTime,errorcode);
                break;
            }
        }else{
            break;
        }
    }
}

bool ProtobufCodecLite::parseFromBuffer(StringPiece buf,
                                     google::protobuf::Message* message){
    //根据buf中的数据，将他们反序列化会message
    return message->ParseFromArray(buf.data(),buf.size());
}
int ProtobufCodecLite::serializeToBuffer(const google::protobuf::Message& message,
                                     Buffer* buf){
    //将message中的数据序列化到buffer厘米里面去
     GOOGLE_DCHECK(message.IsInitialized()) << InitializationErrorMessage("serialize", message);

#if GOOGLE_PROTOBUF_VERSION > 3009002
     int byte_size = google::protobuf::internal::ToIntSize(message.ByteSizeLong());
#else
     int byte_size = message.ByteSize();
#endif
     buf->ensureWritableBytes(byte_size + kChecksumLen);

     uint8_t * start = reinterpret_cast<uint8_t*>(buf->beginWrite());
     uint8_t * end = message.SerializeWithCachedSizesToArray(start);
     if(end - start != byte_size){
        #if GOOGLE_PROTOBUF_VERSION > 3009002
          ByteSizeConsistencyError(byte_size, google::protobuf::internal::ToIntSize(message.ByteSizeLong()), static_cast<int>(end - start));
        #else
          ByteSizeConsistencyError(byte_size, message.ByteSize(), static_cast<int>(end - start));
        #endif
     }
     buf->hasWritten(byte_size);
     return byte_size;
}

namespace {
  const std::string kNoErrorStr = "NoError";
  const std::string kInvalidLengthStr = "InvalidLength";
  const std::string kCheckSumErrorStr = "CheckSumError";
  const std::string kInvalidNameLenStr = "InvalidNameLen";
  const std::string kUnknowMessageTypeStr = "UnknownMessageType";
  const std::string kParseErrorStr = "ParseError";
  const std::string kUnknownErrorStr = "UnknownError";
}

const std::string& ProtobufCodecLite::errorCodeToString(ErrorCode errorCode){
    switch(errorCode){
#define XX(ktype,str)\
        case ktype :\
            return str;
       XX(kNoError,kNoErrorStr);
       XX(kInvalidLength,kInvalidLengthStr);
       XX(kCheckSumError,kCheckSumErrorStr);
       XX(kInvalidNameLen,kInvalidNameLenStr);
       XX(kUnknowMessageType,kUnknowMessageTypeStr);
       XX(kParseError,kParseErrorStr);
#undef XX
        default:
            return "kUnknowErrorStr";
    }
}

void ProtobufCodecLite::defaultErrorCallback(const TcpConnection::ptr& conn,
                                              Buffer* buf,
                                              Timestamp receiveTime,
                                              ErrorCode errorCode){
    JKYI_LOG_ERROR(g_logger) << " ProtobufCodecLite::defaultErrorCallback - "
                             << errorCodeToString(errorCode);
    if(conn && conn->connected()){
        conn->shutdown();
    }
}

int32_t ProtobufCodecLite::asInt32(const void* buf){
    int32_t be32 = 0;
    ::memcpy(&be32,buf,sizeof(be32));
    return toLittleEndian(be32);
}

int32_t ProtobufCodecLite::checksum(const void* buf,int len){
    return static_cast<int32_t>(
            ::adler32(1,static_cast<const Bytef*>(buf),len));
}
//用来对校验码进行检测
bool ProtobufCodecLite::validateChecksum(const void * buf,int len){
    int32_t expectedCheckSum(asInt32(buf + len - kChecksumLen));
    int32_t checkSum = checksum(buf,len - kChecksumLen);
    return checkSum == expectedCheckSum;
}

ProtobufCodecLite::ErrorCode ProtobufCodecLite::parse(const char * buf,int len,
                                             ::google::protobuf::Message* message){
    ErrorCode error = kNoError;
    if(validateChecksum(buf,len)){
       if(memcmp(buf,tag_.c_str(),tag_.size()) == 0){
           //如果tag相同的话
           const char * data = buf + tag_.size();
           int32_t dataLen = len - kChecksumLen - static_cast<int32_t>(tag_.size());
           if(parseFromBuffer(StringPiece(data,dataLen),message)){
               error = kNoError;
           }else{
               error = kParseError;
           }
       }else{
           error = kUnknowMessageType;
       }
    }else{
        error = kCheckSumError;
    }
    return error;
}

