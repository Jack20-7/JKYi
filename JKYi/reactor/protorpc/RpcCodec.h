#ifndef _JKYI_NET_RPC_CODEC_H_
#define _JKYI_NET_RPC_CODEC_H_

#include"JKYi/timestamp.h"
#include"JKYi/reactor/protobuf/ProtobufCodecLite.h"

namespace JKYi{
namespace net{

class Buffer;
class TcpConnection;
typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;

class RpcMessage;
typedef std::shared_ptr<RpcMessage> RpcMessagePtr;
extern const char rpctag[]; // "rpc0"

//format

//Field      Length   Context
//
//size       4        N + 4
//rpc0       4       
//payload    N
//chechsum   4        payload + rpc0
//
//
typedef ProtobufCodecLiteT<RpcMessage,rpctag> RpcCodec;

}//namespace net
}//namespace JKYi

#endif
