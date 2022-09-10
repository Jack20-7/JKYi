#include"frame.h"
#include"JKYi/log.h"

#include<sstream>

namespace JKYi{
namespace http2{

static Logger::ptr g_logger = JKYI_LOG_NAME("system");

std::string FrameHeader::toString()const{
    std::stringstream ss;
    ss << " FrameHeader length = " << length
       << " type = " << FrameTypeToString((FrameType)type)
       << " flags = " << FrameFlagToString(type,flags)
       << " r = "  << (int)r
       << " identifier = " << identifier
       << " ]";

    return ss.str();
}

bool FrameHeader::writeTo(ByteArray::ptr ba){
    try{
        ba->writeFuint32(len_type);
        ba->writeFuint8(flags);
        ba->writeFuint32(r_id);
        return true;
    }catch(...){
        JKYI_LOG_WARN(g_logger) << "write FrameHeader fail, " << toString();
    }
    return true;
}
bool FrameHeader::readFrom(ByteArray::ptr ba){
    try{
        len_type = ba->readFuint32();
        flags = ba->readFuint8();
        r_id = ba->readFuint32();
        return true;
    }catch(...){
        JKYI_LOG_WARN(g_logger) << "read FrameHeader fail, " << toString();
    }
    return false;
}
std::string Frame::toString(){
    std::stringstream ss;
    ss << header.toString();
    if(data){
        ss << data->toString();
    }
    return ss.str();
}
std::string DataFrame::toString()const{
    std::stringstream ss;
    ss << "[DataFrame " ;
    if(pad){
        ss << "pad = " << (uint32_t)pad;
    }
    ss << "data.size = " << data.size();
    ss << "]";
    return ss.str();
}
bool DataFrame::writeTo(ByteArray::ptr ba,const FrameHeader& header){
    try{
        //判断是否存在pad
        if(header.flags & (uint8_t)FrameFlagData::PADDED){
            //如果存在的话
            ba->writeFuint8(pad);
            ba->write(data.c_str(),data.size());
            ba->write(padding.c_str(),padding.size());
        }else{
            //只存在data
            ba->write(data.c_str(),data.size());
        }
        return true;
    }catch(...){
        JKYI_LOG_WARN(g_logger) << "write DataFrame fail, " << toString();
    }
    return false;
}
bool DataFrame::readFrom(ByteArray::ptr ba,const FrameHeader& header){
    //从ba中将数据去除
    try{
        if(header.flags & (uint8_t)FrameFlagData::PADDED){
            pad = ba->readFuint8();
            data.resize(header.length - pad - 1);
            ba->read(&data[0],data.size());
            padding.resize(pad);
            ba->read(&padding[0],pad);
        }else{
            data.resize(header.length);
            ba->read(&data[0],data.size());
        }
        return true;
    }catch(...){
        JKYI_LOG_WARN(g_logger) << "read DataFrame fail, " << toString();
    }
    return false;
}
std::string PriorityFrame::toString()const{
    std::stringstream ss;
    ss << " [PriorityFrame exclusive = " << exclusive
       << " stream_dep = " << stream_dep
       << " weight = " << weight
       << " ]";
    return ss.str();
}

bool PriorityFrame::writeTo(ByteArray::ptr ba,const FrameHeader& header){
    try{
        ba->writeFuint32(e_stream_dep);
        ba->writeFuint8(weight);
        return true;
    }catch(...){
        JKYI_LOG_WARN(g_logger) << "write PriorityFrame fail, " << toString();
    }
    return false;
}
bool PriorityFrame::readFrom(ByteArray::ptr ba,const FrameHeader& header){
    try{
       stream_dep = ba->readFuint32();
       weight = ba->readFuint32();
       return true;
    }catch(...){
        JKYI_LOG_WARN(g_logger) << "read PriorityFrame fail, " << toString();
    }
    return false;
}
std::string HeadersFrame::toString()const{
    std::stringstream ss;
    ss << " [HeadersFrame " ;
    if(pad){
        ss << " pad = " << (uint32_t)pad;
    }
    ss << " data.size() = " << data.size();
    ss << "] ";
    return ss.str();
}
bool HeadersFrame::writeTo(ByteArray::ptr ba,const FrameHeader& header){
    try{
        //判断PADDED是否存在
        if(header.flags & (uint8_t)FrameFlagHeaders::PADDED){
            ba->writeFuint8(pad);
        }
        if(header.flags & (uint8_t)FrameFlagHeaders::PRIORITY){
            priority.writeTo(ba,header);
        }
        if(hpack && !kvs.empty()){
            hpack->pack(kvs,data);
        }
        ba->write(data.c_str(),data.size());
        if(header.flags & (uint8_t)FrameFlagHeaders::PADDED){
            ba->write(padding.c_str(),padding.size());
        }
        return true;
    }catch(...){
        JKYI_LOG_WARN(g_logger) << "write HeadersFrame fail, " << toString();
    }
    return false;
}
bool HeadersFrame::readFrom(ByteArray::ptr ba,const FrameHeader& header){
    try{
        int len = header.length;
        if(header.flags & (uint8_t)FrameFlagHeaders::PADDED){
            pad = ba->readFuint8();
            len -= (pad + 1);
        }
        if(header.flags & (uint8_t)FrameFlagHeaders::PRIORITY){
            priority.readFrom(ba,header);
            len -= 5;
        }
        data.resize(len);
        ba->read(&data[0],data.size());
        if(header.flags & (uint8_t)FrameFlagHeaders::PADDED){
            padding.resize(pad);
            ba->read(&padding[0],pad);
        }
        return true;
    }catch(...){
        JKYI_LOG_WARN(g_logger) << "readFrom HeadersFrame fail, " << toString();
    }
    return false;
}

std::string RstStreamFrame::toString()const{
    std::stringstream ss;
    ss << "[RstStreamFrame error_code = " << error_code << "] ";
    return ss.str();
}
bool RstStreamFrame::writeTo(ByteArray::ptr ba,const FrameHeader& header){
    try{
        ba->writeFuint32(error_code);
        return true;
    }catch(...){
        JKYI_LOG_WARN(g_logger) << "wrietTo RstStreamFrame fail, " << toString();
    }
    return false;
}
bool RstStreamFrame::readFrom(ByteArray::ptr ba,const FrameHeader& header){
    try{
        error_code = ba->readFuint32();
        return true;
    }catch(...){
        JKYI_LOG_WARN(g_logger) << "readFrom RstStreamFrame fail, " <<toString();
    }
    return false;
}
//用来完成Settings向 string 的转化
static std::vector<std::string> s_setting_string = {
    "",
    "HEADER_TABLE_SIZE",
    "ENABLE_PUSH",
    "MAX_CONCURRENT_STRING",
    "INITAL_WINDOW_SIZE",
    "MAX_FRAME_SIZE",
    "MAX_HEADER_LIST_SIZE"
};
std::string SettingsFrame::SettingsToString(Settings s){
    uint32_t idx = (uint32_t)s;
    if(idx <= 0 || idx > 6){
        return "UNKNOW ( " + std::to_string(idx) + ")";
    }
    return s_setting_string[idx];
}

std::string SettingsItem::toString()const{
    std::stringstream ss;
    ss << " [Settings identifiber = " 
       << SettingsFrame::SettingsToString((SettingsFrame::Settings)identifiber)
       << " value = " << value << "] ";
    return ss.str();
}
bool SettingsItem::writeTo(ByteArray::ptr ba){
    try{
        ba->writeFuint16(identifiber);
        ba->writeFuint32(value);
        return true;
    }catch(...){
        JKYI_LOG_WARN(g_logger) << "writeTo SettingsItem fail, " <<toString();
    }
    return false;
}
bool SettingsItem::readFrom(ByteArray::ptr ba){
    identifiber = ba->readFuint16();
    value = ba->readFuint32();
    return true;
}
std::string SettingsFrame::toString()const{
    std::stringstream ss;
    ss << "[SettingsFrame size = " << items.size() << " items = [";
    for(auto& i : items){
        ss << i.toString();
    }
    ss << " ]]";
    return ss.str();
}
bool SettingsFrame::writeTo(ByteArray::ptr ba,const FrameHeader& header){
    try{
        for(auto& i : items){
            i.writeTo(ba);
        }
        return true;
    }catch(...){
        JKYI_LOG_WARN(g_logger) << "writeTo SettingsFrame fail, " << toString();
    }
    return false;
}
bool SettingsFrame::readFrom(ByteArray::ptr ba,const FrameHeader& header){
    try{
        uint32_t size = header.length / sizeof(SettingsItem);
        items.resize(size);
        for(uint32_t i = 0;i < size; ++i){
            items[i].readFrom(ba);
        }
        return true;
    }catch(...){
        JKYI_LOG_WARN(g_logger) << " readFrom SettingsFrame fail, " << toString();
    }
    return false;
}

std::string PushPromisedFrame::toString()const{
    std::stringstream ss;
    ss << " [PushPromisedFrame ";
    if(pad){
        ss << " pad = " << pad;
    }
    ss << " data.size = " << data.size();
    ss << " ] ";
    return ss.str();
}
bool PushPromisedFrame::writeTo(ByteArray::ptr ba,const FrameHeader& header){
    try{
        if(header.flags & (uint8_t)FrameFlagPromise::PADDED){
            ba->writeFuint8(pad);
            ba->writeFuint32(r_stream_id);
            ba->write(data.c_str(),data.size());
            ba->write(padding.c_str(),padding.size());
        }else{
            ba->writeFuint32(r_stream_id);
            ba->write(data.c_str(),data.size());
        }
    }catch(...){
        JKYI_LOG_WARN(g_logger) << "write RstPromisedFrame fail, " << toString();
    }
    return false;
}
bool PushPromisedFrame::readFrom(ByteArray::ptr ba,const FrameHeader& header){
    try{
        if(header.flags & (uint8_t)FrameFlagPromise::PADDED){
            pad = ba->readFuint8();
            r_stream_id = ba->readFuint32();
            data.resize(header.length - pad - 5);
            ba->read(&data[0],data.size());
            padding.resize(pad);
            ba->read(&padding[0],padding.size());
        }else{
            r_stream_id = ba->readUint32();
            data.resize(header.length - 4);
            ba->read(&data[0],data.size());
        }
        return true;
    }catch(...){
        JKYI_LOG_WARN(g_logger) << "readFrom PushPromisedFrame fail, " << toString();
    }
    return false;
}

std::string PingFrame::toString()const {
    std::stringstream ss;
    ss << "[PingFrame uint64 = " << uint64 << "]";
    return ss.str();
}
bool PingFrame::writeTo(ByteArray::ptr ba,const FrameHeader& header){
    try{
        ba->write(data,8);
        return true;
    }catch(...){
        JKYI_LOG_WARN(g_logger) << "writeTo PingFrame fail, " <<toString();
    }
    return false;
}
bool PingFrame::readFrom(ByteArray::ptr ba,const FrameHeader& header){
    try{
        ba->read(data,8);
    }catch(...){
        JKYI_LOG_WARN(g_logger) << "readFrom PingFrame fail, " <<toString();
    }
    return false;
}
std::string GoAwayFrame::toString()const{
    std::stringstream ss;
    ss << " [GoAwayFrame r = " << r
       << " last_stream_id = " <<last_stream_id
       << " error_code = " << error_code 
       << " data.size = " << data.size()
       << " ]";
    return ss.str();
}
bool GoAwayFrame::writeTo(ByteArray::ptr ba,const FrameHeader& header){
    try{
        ba->writeFuint32(r_last_stream_id);
        ba->writeFuint32(error_code);
        if(!data.empty()){
            ba->write(data.c_str(),data.size());
        }
        return true;
    }catch(...){
        JKYI_LOG_WARN(g_logger) << "writeTo GoAwayFrame fail, " << toString();
    }
    return false;
}
bool GoAwayFrame::readFrom(ByteArray::ptr ba,const FrameHeader& header){
    try{
        r_last_stream_id = ba->readFuint32();
        error_code = ba->readFuint32();
        if(header.length){
            data.resize(header.length - 8);
            ba->read(&data[0],data.size());
        }
        return true;
    }catch(...){
        JKYI_LOG_WARN(g_logger) << "readFrom GoAwayFrame fail, " << toString();
    }
    return false;
}

std::string WindowUpdateFrame::toString()const{
    std::stringstream ss;
    ss << " [WindowUpdateFrame r = " << r
       << " increment = " << increment
       << " ]";
    return ss.str();
}
bool WindowUpdateFrame::writeTo(ByteArray::ptr ba,const FrameHeader& header){
    try{
        ba->writeFuint32(r_increment);
        return true;
    }catch(...){
        JKYI_LOG_WARN(g_logger) << "writeTo WindowUpdateFrame fail, " << toString();
    }
    return false;
}
bool WindowUpdateFrame::readFrom(ByteArray::ptr ba,const FrameHeader& header){
    try{
        r_increment = ba->readFuint32();
        return true;
    }catch(...){
        JKYI_LOG_WARN(g_logger) << "readFrom WindowUpdateFrame fail, " << toString();
    }
    return false;
}

static const std::vector<std::string> s_frame_types = {
    "DATA",
    "HEADERS",
    "PRIORITY",
    "RST_STREAM",
    "SETTINGS",
    "PUSH_PROMISE",
    "PING",
    "GOAWAY",
    "WINDOW_UPDATE",
    "CONTINUATION",
};
std::string FrameTypeToString(FrameType type){
    uint8_t v = (uint8_t)type;
    if(v > 9){
        return "UNKNOW(" + std::to_string(v) + ")";
    }
    return s_frame_types[v];
}

#define XX(ff,str)\
    std::string res;\
    if((uint8_t)flag & (uint8_t)ff){\
        res = str;\
    }
#define XX_IF(ff,str)\
    if((uint8_t)flag & (uint8_t)ff){\
        if(!res.empty()){\
            res += "|";\
        }\
        res += str;\
    }
#define XX_END()\
    if(res.empty()){\
        if((uint8_t)flag){\
            res = "UNKNOW(" + std::to_string((uint32_t)flag) + ")";\
        }else{\
            res = "0";\
        }\
    }\
    return res;
std::string FrameFlagDataToString(FrameFlagData flag){
    XX(FrameFlagData::END_STREAM,"END_STREAM");
    XX_IF(FrameFlagData::PADDED,"PADDED");
    XX_END();
}

std::string FrameFlagHeadersToString(FrameFlagHeaders flag){
    XX(FrameFlagHeaders::END_STREAM,"END_STREAM");
    XX_IF(FrameFlagHeaders::END_HEADERS,"END_HEADERS");
    XX_IF(FrameFlagHeaders::PADDED,"PADDED");
    XX_IF(FrameFlagHeaders::PRIORITY,"PRIORITY");
    XX_END();
}
std::string FrameFlagSettingsToString(FrameFlagSettings flag){
    XX(FrameFlagSettings::ACK,"ACK");
    XX_END();
}
std::string FrameFlagPingToString(FrameFlagPing flag){
    XX(FrameFlagPing::ACK,"ACK");
    XX_END();
}
std::string FrameFlagContinuationToString(FrameFlagContinuation flag){
    XX(FrameFlagContinuation::END_HEADERS,"END_HEADERS");
    XX_END();
}
std::string FrameFlagPromiseToString(FrameFlagPromise flag){
    XX(FrameFlagPromise::END_HEADERS,"END_HEADERS");
    XX_IF(FrameFlagPromise::PADDED,"PADDED");
    XX_END();
}

std::string FrameFlagToString(uint8_t type,uint8_t flag){
    switch((FrameType)type){
        case FrameType::DATA:
            return FrameFlagDataToString((FrameFlagData)flag);
        case FrameType::HEADERS:
            return FrameFlagHeadersToString((FrameFlagHeaders)flag);
        case FrameType::SETTINGS:
            return FrameFlagSettingsToString((FrameFlagSettings)flag);
        case FrameType::PING:
            return FrameFlagPingToString((FrameFlagPing)flag);
        case FrameType::CONTINUATION:
            return FrameFlagContinuationToString((FrameFlagContinuation)flag);
        default:
            if(flag){
                return "UNKNOW(" + std::to_string((uint32_t)flag) + ")";
            }else{
                return "0";
            }
    }
}
std::string FrameRToString(FrameR r){
    if(r == FrameR::SET){
        return "SET";
    }
        return "UNSET";
}

//该函数的左右就是将从stream中读出的数据分装成一个Frame进行反追
Frame::ptr FrameCodec::parseFrom(Stream::ptr stream){
    try{
       Frame::ptr frame = std::make_shared<Frame>();
       ByteArray::ptr ba(new ByteArray());
       //首先从stream中将FrameHeader读出，然后放入到ByteArray里面取
       int rt = stream->readFixSize(ba,FrameHeader::SIZE);
       if(rt <= 0){
           JKYI_LOG_INFO(g_logger) << "recv Frame header fail,rt = " << rt << "   "
                                   << strerror(errno);
       }
       ba->setPosition(0);
       if(!frame->header.readFrom(ba)){
           JKYI_LOG_INFO(g_logger) << "parse frame header fail";
           return nullptr;
       }
       if(frame->header.length > 0){
           ba->setPosition(0);
           int rt = stream->readFixSize(ba,frame->header.length);
           if(rt <= 0){
               JKYI_LOG_INFO(g_logger) << "recv frame body fail,rt =" << rt
                                       << "  " << strerror(errno);
               return nullptr;
           }
           ba->setPosition(0);
       }
       //带目前位置，frame的body部分也读入到了ba里面去了
       switch((FrameType)frame->header.type){
           case FrameType::DATA: 
               {
                  frame->data = std::make_shared<DataFrame>();
                  if(!frame->data->readFrom(ba,frame->header)){
                      JKYI_LOG_INFO(g_logger) << "parse DataFrame fail ";
                      return nullptr;
                  }
               }
               break;
           case FrameType::HEADERS:
               {
                   frame->data = std::make_shared<HeadersFrame>();
                   if(!frame->data->readFrom(ba,frame->header)){
                       JKYI_LOG_INFO(g_logger) << "parse HeaderFrame fail ";
                       return nullptr;
                   }
               }
               break;
           case FrameType::PRIORITY:
               {
                   frame->data = std::make_shared<PriorityFrame>();
                   if(!frame->data->readFrom(ba,frame->header)){
                       JKYI_LOG_INFO(g_logger) << "parse PriorityFrame fail ";
                       return nullptr;
                   }
               }
               break;
           case FrameType::RST_STREAM:
               {
                   frame->data = std::make_shared<RstStreamFrame>();
                   if(!frame->data->readFrom(ba,frame->header)){
                       JKYI_LOG_INFO(g_logger) << " parse RstStreamFrame fail ";
                       return nullptr;
                   }
               }
               break;
           case FrameType::SETTINGS:
               {
                   frame->data = std::make_shared<SettingsFrame>();
                   if(!frame->data->readFrom(ba,frame->header)){
                       JKYI_LOG_INFO(g_logger) << " parse SettingsFrame fail ";
                       return nullptr;
                   }
               }
               break;
           case FrameType::PUSH_PROMISE:
               {
                   frame->data = std::make_shared<PushPromisedFrame>();
                   if(!frame->data->readFrom(ba,frame->header)){
                       JKYI_LOG_INFO(g_logger) << " parse PushPromisedFrame fail ";
                       return nullptr;
                   }
               }
               break;
           case FrameType::PING:
               {
                   frame->data = std::make_shared<PingFrame>();
                   if(!frame->data->readFrom(ba,frame->header)){
                       JKYI_LOG_INFO(g_logger) << " parse PingFrame fail";
                       return nullptr;
                   }
               }
               break;
           case FrameType::GOAWAY:
               {
                   frame->data = std::make_shared<GoAwayFrame>();
                   if(!frame->data->readFrom(ba,frame->header)){
                       JKYI_LOG_INFO(g_logger) << " parse GoAwayFrame fail";
                       return nullptr; 
                   }
               }
               break;
           case FrameType::WINDOW_UPDATE:
               {
                   frame->data = std::make_shared<WindowUpdateFrame>();
                   if(!frame->data->readFrom(ba,frame->header)){
                       JKYI_LOG_INFO(g_logger) << " parse WindowUpdateFrame fail ";
                       return nullptr;
                   }
               }
               break;
           case FrameType::CONTINUATION:
               {
               }
               break;
           default:
               {
                   JKYI_LOG_WARN(g_logger) << "invalid FrameType: " 
                                           <<  (uint32_t)frame->header.type;
                   return nullptr;
               }
               break;
       }
       return frame;
    }catch(std::exception & e){
        JKYI_LOG_ERROR(g_logger) << " FrameCodec parseFrame except: " << e.what();
    }catch(...){
        JKYI_LOG_ERROR(g_logger) << " FrameCodec parseFrom  except";
    }
    return nullptr;
}


int32_t FrameCodec::serializeTo(Stream::ptr stream,Frame::ptr frame){
    JKYI_LOG_DEBUG(g_logger) << " serailizeTo " << frame->toString();
    ByteArray::ptr ba(new ByteArray);
    frame->header.writeTo(ba);
    if(frame->data){
        if(!frame->data->writeTo(ba,frame->header)){
            JKYI_LOG_ERROR(g_logger) << " FrameCodec serializeTo fail,type = " 
                                     <<  FrameTypeToString((FrameType)frame->header.type);
            return -1;
        }
        int pos = ba->getPosition();
        ba->setPosition(0);
        //重新计算一下frame中data的长度
        frame->header.length = pos - FrameHeader::SIZE;
        frame->header.writeTo(ba);
    }
    ba->setPosition(0);
    int rt = stream->writeFixSize(ba,ba->getReadSize());
    if(rt <= 0){
        JKYI_LOG_ERROR(g_logger) << " FrameCodec serializeTo fail,rt = " << rt
                                 << " errno = " << errno 
                                 << " errstr = " << strerror(errno);
        return -2;
    }
    return ba->getSize();
}

}
}

