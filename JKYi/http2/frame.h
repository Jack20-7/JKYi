#ifndef _JKYI_FRAME_H_
#define _JKYI_FRAME_H_

#include"JKYi/bytearray.h"
#include"JKYi/stream.h"
#include"hpack.h"

namespace JKYi{
namespace http2{
//设置结构体内存按1进行对齐
#pragma pack(push)
#pragma pack(1)

//帧的类型
enum class FrameType{
    //数据帧
    DATA           = 0x0,
    //报头帧
    HEADERS        = 0x1,
    //优先级帧
    PRIORITY       = 0x2,
    //流终止帧
    RST_STREAM     = 0X3,
    //设置帧
    SETTINGS       = 0x4,
    //推送帧
    PUSH_PROMISE   = 0X5,
    PING           = 0X6,
    //用来发起关闭连接的请求
    GOAWAY         = 0x7,
    //窗口更新帧,用来执行流量控制功能
    WINDOW_UPDATE  = 0X8,
    //延续帧
    CONTINUATION   = 0X9
};

//Http2.0中帧的格式分为
/*
HTTP2 frame格式
+-----------------------------------------------+
|                 Length (24)                   |
+---------------+---------------+---------------+
|   Type (8)    |   Flags (8)   |
+-+-------------+---------------+-------------------------------+
|R|                 Stream Identifier (31)                      |
+=+=============================================================+
|                   Frame Payload (0...)                      ...
+---------------------------------------------------------------+
*/
//数据帧中的flag的类型
enum class FrameFlagData{
    //表示本帧是当前流的最后一帧
    END_STREAM = 0x1,
    //代表存在padding
    PADDED     = 0x8
};

//Header帧flag的类型
enum class FrameFlagHeaders{
    END_STREAM     = 0x1,
    END_HEADERS    = 0x4,
    PADDED         = 0x8,
    PRIORITY       = 0x20
};

//setting帧的falg类型
enum class FrameFlagSettings{
    ACK            = 0x1
};

enum class FrameFlagPing{
    ACK            = 0x1
};

enum class FrameFlagContinuation{
    END_HEADERS    = 0x4
};

enum class FrameFlagPromise{
    END_HEADERS    = 0x4,
    PADDED         = 0x8
};

//frame中R这个编制为的类型
//发送时必须置为0,接受时可以忽略
enum class FrameR{
    UNSET    = 0x0,
    SET      = 0x1
};
/*
HTTP2 frame 格式
+-----------------------------------------------+
|                 Length (24)                   |
+---------------+---------------+---------------+
|   Type (8)    |   Flags (8)   |
+-+-------------+---------------+-------------------------------+
|R|                 Stream Identifier (31)                      |
+=+=============================================================+
|                   Frame Payload (0...)                      ...
+---------------------------------------------------------------+
*/
//用来表示frame首部的结构体类型
struct FrameHeader{
    //frame首部固定九个字节
    static const uint32_t SIZE = 9;
    typedef std::shared_ptr<FrameHeader> ptr;
    union{
        struct {
            uint8_t type;
            //表示的是frame携带的负载的长度
            uint32_t length : 24;
        };
        uint32_t len_type = 0;
    };
    uint8_t flags = 0;
    union{
        struct {
            uint32_t identifier : 31;
            uint32_t r : 1;
        };
        uint32_t r_id = 0;
    };

    std::string toString()const;
    bool writeTo(ByteArray::ptr ba);
    bool readFrom(ByteArray::ptr ba);
};

//作为frame data部分的抽象基类
class IFrame{
public:
    typedef std::shared_ptr<IFrame> ptr;

    virtual ~IFrame(){}
    virtual std::string toString()const = 0;
    virtual bool writeTo(ByteArray::ptr ba,const FrameHeader& header) = 0;
    virtual bool readFrom(ByteArray::ptr ba,const FrameHeader& header) = 0;
};
//这个就是真正的帧的类型
struct Frame{
    typedef std::shared_ptr<Frame> ptr;

    FrameHeader header;
    IFrame::ptr data;

    std::string toString();
};

//下面就是根据帧的类型不同，data部分的不同类型
//数据帧
/*
 +---------------+
 |Pad Length? (8)|
 +---------------+-----------------------------------------------+
 |                            Data (*)                         ...
 +---------------------------------------------------------------+
 |                           Padding (*)                       ...
 +---------------------------------------------------------------+ 
*/
struct DataFrame:public IFrame{
    typedef std::shared_ptr<DataFrame> ptr;
    //该字段出现是有条件的，只有在flag上设置来才会出现，它的值表示padding的长度
    uint8_t pad = 0;                        //Flag & FrameFlagData::PADDED
    std::string data;
    std::string padding; 

    std::string toString()const override;
    bool writeTo(ByteArray::ptr ba,const FrameHeader& header) override;
    bool readFrom(ByteArray::ptr ba,const FrameHeader& header) override;
};

//优先级帧 data部分
/*
 +-+-------------------------------------------------------------+
 |E|                  Stream Dependency (31)                     |
 +-+-------------+-----------------------------------------------+
 |   Weight (8)  |
 +-+-------------+
*/
struct PriorityFrame:public IFrame{
    typedef std::shared_ptr<PriorityFrame> ptr;
    static const uint32_t SIZE = 5;
    union{
        struct {
            //要给那一个流指定优先级
            uint32_t stream_dep : 31;
            uint32_t exclusive : 1;
        };
        uint32_t e_stream_dep = 0;
    };
    //指定的优先级，也就是权重
    uint8_t weight = 0;

    std::string toString()const override;
    bool writeTo(ByteArray::ptr ba,const FrameHeader& header) override;
    bool readFrom(ByteArray::ptr ba,const FrameHeader& header) override;
};

//Header帧
/*
 +---------------+
 |Pad Length? (8)|
 +-+-------------+-----------------------------------------------+
 |E|                 Stream Dependency? (31)                     |
 +-+-------------+-----------------------------------------------+
 |  Weight? (8)  |
 +-+-------------+-----------------------------------------------+
 |                   Header Block Fragment (*)                 ...
 +---------------------------------------------------------------+
 |                           Padding (*)                       ...
 +---------------------------------------------------------------+
 */
struct HeadersFrame : public IFrame{
    typedef std::shared_ptr<HeadersFrame> ptr;
    uint8_t pad = 0;        //flag & FrameFlagHeaders::PADDED
    PriorityFrame priority; //flag & FrameFlagHeaders::PRIORITY
    std::string data;
    std::string padding;
    HPack::ptr hpack;
    //用来存储首部字段
    std::vector<std::pair<std::string,std::string>> kvs;

    std::string toString()const override;
    bool writeTo(ByteArray::ptr ba,const FrameHeader& header) override;
    bool readFrom(ByteArray::ptr ba,const FrameHeader& header) override;
};

//终止帧,用来终止流 或者 在流出现错误时返回对于的错误码
/*
 +---------------------------------------------------------------+
 |                        Error Code (32)                        |
 +---------------------------------------------------------------+
*/
struct RstStreamFrame: public IFrame{
    typedef std::shared_ptr<RstStreamFrame> ptr;
    static const uint32_t SIZE = 4;
    uint32_t error_code = 0;

    std::string toString()const override;
    bool writeTo(ByteArray::ptr ba,const FrameHeader& header) override;
    bool readFrom(ByteArray::ptr ba,const FrameHeader& header) override;
};

//参数设置帧
/*
 +-------------------------------+
 |       Identifier (16)         |
 +-------------------------------+-------------------------------+
 |                        Value (32)                             |
 +---------------------------------------------------------------+
*/
struct SettingsItem{
    SettingsItem(uint16_t id = 0,uint32_t v = 0)
        :identifiber(id),
         value(v){
     }
    //要设置的参数项，取值的话是从Settings里面去取
    uint16_t identifiber = 0;
    uint32_t value = 0;

    std::string toString()const;
    bool writeTo(ByteArray::ptr ba);
    bool readFrom(ByteArray::ptr ba);
};
struct SettingsFrame:public IFrame{
    typedef std::shared_ptr<SettingsFrame> ptr;
    //设置的参数类型
    enum class Settings{
        HEADER_TABLE_SIZE        =  0x1,
        ENABLE_PUSH              =  0x2,
        MAX_CONCURRENT_STRING    =  0x3,
        INITAL_WINDOW_SIZE       =  0x4,
        MAX_FRAME_SIZE           =  0x5,
        MAX_HEADER_LIST_SIZE     =  0x6
    };
    static std::string SettingsToString(Settings s);

    std::string toString()const override;
    bool writeTo(ByteArray::ptr ba,const FrameHeader& header) override;
    bool readFrom(ByteArray::ptr ba,const FrameHeader& header) override;

    std::vector<SettingsItem> items;
};
// 推送帧,用于服务器向用户来推送资源
/*
 +---------------+
 |Pad Length? (8)|
 +-+-------------+-----------------------------------------------+
 |R|                  Promised Stream ID (31)                    |
 +-+-----------------------------+-------------------------------+
 |                   Header Block Fragment (*)                 ...
 +---------------------------------------------------------------+
 |                           Padding (*)                       ...
 +---------------------------------------------------------------+
*/
struct PushPromisedFrame : public IFrame{
    typedef std::shared_ptr<PushPromisedFrame> ptr;
    uint8_t pad = 0;
    union{
        struct {
            uint32_t stream_id : 31;
            uint32_t r : 1;
        };
        uint32_t r_stream_id = 0;
    };
    std::string data;
    std::string padding;

    std::string toString()const override;
    bool writeTo(ByteArray::ptr ba,const FrameHeader& header) override;
    bool readFrom(ByteArray::ptr ba,const FrameHeader& header) override;
};

//ping帧
/*
 +---------------------------------------------------------------+
 |                                                               |
 |                      Opaque Data (64)                         |
 |                                                               |
 +---------------------------------------------------------------+
 */
struct PingFrame : public IFrame{
    typedef std::shared_ptr<PingFrame> ptr;
    static const uint32_t SIZE = 8;

    union{
        uint8_t data[8];
        uint64_t uint64 = 0;
    };

    std::string toString()const override;
    bool writeTo(ByteArray::ptr ba,const FrameHeader& header) override;
    bool readFrom(ByteArray::ptr ba,const FrameHeader& header) override;
};

//GoAway帧，用于发起关闭连接，或者警示严重的错误
/*
 +-+-------------------------------------------------------------+
 |R|                  Last-Stream-ID (31)                        |
 +-+-------------------------------------------------------------+
 |                      Error Code (32)                          |
 +---------------------------------------------------------------+
 |                  Additional Debug Data (*)                    |
 +---------------------------------------------------------------+
 */
struct GoAwayFrame : public IFrame{
    typedef std::shared_ptr<GoAwayFrame> ptr;
    union{
        struct {
            uint32_t last_stream_id : 31;
            uint32_t r : 1;
        };
        uint32_t r_last_stream_id = 0;
    };
    uint32_t error_code = 0;
    std::string data;

    std::string toString()const override;
    bool writeTo(ByteArray::ptr ba,const FrameHeader& header) override;
    bool readFrom(ByteArray::ptr ba,const FrameHeader& header) override;
};
//窗口更新帧，用于进行流量控制
/*
 +-+-------------------------------------------------------------+
 |R|              Window Size Increment (31)                     |
 +-+-------------------------------------------------------------+
 */
struct WindowUpdateFrame : public IFrame{
    typedef std::shared_ptr<WindowUpdateFrame> ptr;
    static const uint32_t SIZE = 4;
    union{
        struct {
            uint32_t increment : 31;
            uint32_t r : 1;
        };
        uint32_t r_increment = 0;
    };

    std::string toString()const override;
    bool writeTo(ByteArray::ptr ba,const FrameHeader& header) override;
    bool readFrom(ByteArray::ptr ba,const FrameHeader& header) override;
};

class FrameCodec{
public:
    typedef std::shared_ptr<FrameCodec> ptr;

    //根据stream中的数据创建出一个Frame
    Frame::ptr parseFrom(Stream::ptr stream);
    //将frame写入到Stream里面去
    int32_t serializeTo(Stream::ptr stream,Frame::ptr frame);
};

std::string FrameTypeToString(FrameType type);

std::string FrameFlagDataToString(FrameFlagData flag);
std::string FrameFlagHeadersToString(FrameFlagHeaders flag);
std::string FrameFlagSettingsToString(FrameFlagSettings flag);
std::string FrameFlagPingToString(FrameFlagPing flag);
std::string FrameFlagContinuationToString(FrameFlagContinuation flag);
std::string FrameFlagPromiseToString(FrameFlagPromise flag);
std::string FrameFlagToString(uint8_t type,uint8_t flag);

std::string FrameRToString(FrameR r);

#pragma pack(pop)

}
}
#endif
