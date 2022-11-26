#include"JKYi/reactor/protorpc/RpcCodec.h"
#include"JKYi/reactor/Logging.h"
#include"JKYi/endian.h"
#include"JKYi/reactor/TcpConnection.h"

#include"JKYi/reactor/protorpc/rpc.pb.h"
#include"JKYi/reactor/protorpc/google-inl.h"

namespace {
    int ProtobufVersionCheck(){
        GOOGLE_PROTOBUF_VERIFY_VERSION;
        return 0;
    }
    int dummy __attribute__ ((unused)) = ProtobufVersionCheck();
}

namespace JKYi{
namespace net{
   const char rpctag = "RPC0";
}
}
