#ifndef _JKYI_NET_FILEUTIL_H_
#define _JKYI_NET_FILEUTIL_H_

#include"JKYi/noncopyable.h"
#include"JKYi/reactor/StringPiece.h"
#include<sys/types.h>

namespace JKYi{
namespace net{
class AppendFile : public Noncopyable{
public:
    explicit AppendFile(const std::string& filename);
    ~AppendFile();

    void append(const char* logline,size_t len);

    void flush();
    off_t writtenBytes()const { return writtenBytes_; }
private:
    size_t write(const char* logfile,size_t len);

    FILE* fp_;
    char buffer_[64 * 1024];
    off_t writtenBytes_;
};

}
}
#endif
