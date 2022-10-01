#ifndef _JKYI_PROTOBUF_BUFFERSTREAM_H_
#define _JKYI_PROTOBUF_BUFFERSTREAM_H_

#include"JKYi/reactor/Buffer.h"
#include"google/protobuf/io/zero_copy_stream.h"

namespace JKYi{
namespace net{

class BufferOutputStream : public google::protobuf::io::ZeroCopyOutputStream{
public:
    BufferOutputStream(Buffer* buf)
        :buffer_(buf),
         originalSize_(buf->readableBytes()){}
    virtual bool Next(void ** data,int * size){
        buffer_->ensureWritableBytes(4096);
        *data = buffer_->beginWrite();
        *size = static_cast<int>(buffer_->writableBytes());
        buffer_->hasWritten(*size);
        return true;
    }
    virtual void BackUp(int count){
        buffer_->unwrite(count);
    }
    virtual int64_t ByteCount()const{
        return buffer_->readableBytes() - originalSize_;
    }
private:
    Buffer* buffer_;
    size_t originalSize_;
};

}
}

#endif
