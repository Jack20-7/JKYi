#include"JKYi/reactor/examples/memcached/server/Session.h"
#include"JKYi/reactor/examples/memcached/server/MemcacheServer.h"
#include"JKYi/reactor/Logging.h"
#include"JKYi/reactor/Buffer.h"
#include"JKYi/reactor/StringPiece.h"

#include<string>
#ifdef HAVE_TCMALLOC
#include<gperftools/malloc_extension.h>
#endif

//判断是否是二进制协议
static bool isBinaryProtocol(uint8_t firstByte){
    //0x80 = 0x 1000 0000
    return firstByte == 0x80;
}

const int kLongestKeySize = 250;
std::string Session::kLongestKey(kLongestKeySize,'x');

//自定义的分词规则
template<class InputIterator ,class Token>
bool Session::SpaceSeparator::operator() (InputIterator& next,
                                          InputIterator end,Token& tok){
    while(next != end && *next == ' '){
        next++;
    }
    if(next == end){
        tok.clear();
        return false;
    }
    //表示遇到了有效字符
    InputIterator start(next);
    ///寻找下一个空格的位置
    const char* sp = static_cast<const char*>(memchr(start,' ',end - start));
    if(sp){
        tok.set(start,static_cast<int>(sp - start));
        next = sp;
    }else{
        tok.set(start,static_cast<int>(end - start));
        next = end;
    }
    return true;
}

struct Session::Reader{
public:
    Reader(Tokenizer::iterator& begin,Tokenizer::iterator end)
        :first_(begin),
         last_(end){
     }

    //通过自定义的分词规则，取出一个字符串，然后将他转化为ull
    template<class T>
    bool read(T* val){
        if(first_ == last_){
            return false;
        }
        char* end = NULL;
        //strtoull是用来将字符串转化为 unsigned long long
        uint64_t x = strtoull((*first_).data(),&end,10);
        if(end ==(*first_).end()){
            *val = static_cast<T>(x);
            ++first_;
            return true;
        }
        return false;
    }
private:
    Tokenizer::iterator first_;
    Tokenizer::iterator last_;
};

void Session::onMessage(const JKYi::net::TcpConnectionPtr& conn,
                        JKYi::net::Buffer* buf,
                        JKYi::net::Timestamp receiveTime){
    const size_t initialReadable = buf->readableBytes();
    while(buf->readableBytes() > 0){
        if(state_ == kNewCommand){
            if(protocol_ == kAuto){
                assert(bytesRead_ == 0);
                protocol_ = isBinaryProtocol(buf->peek()[0]) ? kBinary : kAscii;
            }
            assert(protocol_ == kAscii || protocol_ == kBinary);
            if(protocol_ == kBinary){
                //TODO SURRPORT Binary protocol
            }else{
                //protocol == Ascii
                const char* crlf = buf->findCRLF();
                if(crlf){
                    int len = static_cast<int>(crlf - buf->peek());
                    JKYi::net::StringPiece request(buf->peek(),len);
                    if(processRequest(request)){
                        resetRequest();
                    }
                    buf->retrieveUntil(crlf + 2);
                }else{
                    if(buf->readableBytes() > 1024){
                        conn_->shutdown();
                    }
                    break;
                }
            }
        }else if(state_ == kReceiveValue){
            receiveValue(buf);
        }else if(state_ == kDiscardValue){
            discardValue(buf);
        }else{
            assert(false);
        }
    }
    bytesRead_ += initialReadable - buf->readableBytes();
}

void Session::receiveValue(JKYi::net::Buffer* buf){
    assert(currItem_.get());
    assert(state_ == kReceiveValue);

    const size_t avail = std::min(buf->readableBytes(),currItem_->neededBytes());
    assert(currItem_.unique());
    currItem_->append(buf->peek(),avail);
    buf->retrieve(avail);
    if(currItem_->neededBytes() == 0){
        if(currItem_->endsWithCRLF()){
            bool exists = false;
            if(owner_->storeItem(currItem_,policy_,&exists)){
                reply("STORED\r\n");
            }else{
                if(policy_ == Item::kCas){
                    if(exists){
                        reply("EXISTS\r\n");
                    }else{
                        reply("NOT_FOUND\r\n");
                    }
                }else{
                    reply("NOT_STORED\r\n");
                }
            }
        }else{
            reply("CLIENT_ERROR bad data chunk\r\n");
        }
        resetRequest();
        state_ = kNewCommand;
    }
}

void Session::discardValue(JKYi::net::Buffer* buf){
    assert(!currItem_);
    assert(state_ == kDiscardValue);
    if(buf->readableBytes() < bytesToDiscard_){
        bytesToDiscard_ -= buf->readableBytes();
        buf->retrieveAll();
    }else{
        buf->retrieve(bytesToDiscard_);
        bytesToDiscard_ = 0;
        resetRequest();
        state_ = kNewCommand;
    }
}

//对客户端发送来的命令进行处理
bool Session::processRequest(JKYi::net::StringPiece request){
    assert(command_.empty());
    assert(!noreply_);
    assert(policy_  == Item::kInvalid);
    assert(!currItem_);
    assert(bytesToDiscard_ == 0);
    ++requestsProcessed_;

    //check noreply
    if(request.size() >= 8){
        //取出发送来的请求的最后8个字节
        JKYi::net::StringPiece end(request.end() - 8,8);
        if(end == "noreply"){
            noreply_ = true;
            request.remove_suffix(8);
        }
    }

    SpaceSeparator sep;
    Tokenizer tok(request.begin(),request.end(),sep);
    Tokenizer::iterator begin = tok.begin();
    if(begin == tok.end()){
        reply("ERROR\r\n");
        return true;
    }
    (*begin).CopyToString(&command_);
    ++begin;
    if(command_ == "set" || command_ == "add" || command_ == "replace"
                         || command_ == "append" || command_ == "prepend" 
                         || command_ == "cas"){
        return doUpdate(begin,tok.end());
    }else if(command_ == "get" || command_ == "gets"){
        bool cas = (command_ == "gets");
        while(begin != tok.end()){
            JKYi::net::StringPiece key = *begin;
            bool good = (key.size() <= kLongestKeySize);
            if(!good){
                reply("CLIENT_ERROR bad command line format\r\n");
                return true;
            }
            needle_->resetKey(key);
            ConstItemPtr item = owner_->getItem(needle_);
            ++begin;
            if(item){
                item->output(&outputBuf_,cas);
            }
        }
        outputBuf_.append("END\r\n");
        if(conn_->outputBuffer()->writableBytes() > 65535 + outputBuf_.readableBytes()){
            LOG_ERROR << "shrink output buffer from " 
                      << conn_->outputBuffer()->internalCapacity();
            conn_->outputBuffer()->shrink(65535 + outputBuf_.readableBytes());
        }
        conn_->send(&outputBuf_);
    }else if(command_ == "delete"){
        doDelete(begin,tok.end());
    }else if(command_ == "version"){
#ifdef HAVE_TCMALLOC
        reply("VERSION 0.01 JKYi with tcmalloc\r\n");
#else
        reply("VERSION 0.01 JKYi\r\n");
#endif
    }
#ifdef HAVE_TCMALLOC
    else if(command_ == "memstat"){
        char buf[1024 * 64];
        MallocExtension::instance()->GetStats(buf,sizeof buf);
        reply(buf);
    }
#endif
    else if(command_ == "quit"){
        conn_->shutdown();
    }else if(command_ == "shutdown"){
        conn_->shutdown();
        owner_->stop();
    }else{
        reply("ERROR\r\n");
        LOG_INFO << "Unknown command: " << command_;
    }
    return true;
}

void Session::resetRequest(){
    command_.clear();
    noreply_ = false;
    policy_ = Item::kInvalid;
    currItem_ .reset();
    bytesToDiscard_ = 0;
}

void Session::reply(JKYi::net::StringPiece msg){
    if(!noreply_){
        conn_->send(msg.data(),msg.size());
    }
}

bool Session::doUpdate(Session::Tokenizer::iterator& begin,
                       Session::Tokenizer::iterator end){
    if(command_ == "set"){
       policy_ = Item::kSet;
    }else if(command_ == "add"){
        policy_ = Item::kAdd;
    }else if(command_ == "replace"){
        policy_ = Item::kReplace;
    }else if(command_ == "append"){
        policy_ = Item::kAppend;
    }else if(command_ == "prepend"){
        policy_ = Item::kPrepend;
    }else if(command_ == "cas"){
        policy_ = Item::kCas;
    }else{
        assert(false);
    }
    JKYi::net::StringPiece key = (*begin);
    ++begin;
    bool good = (key.size() <= kLongestKeySize);

    uint32_t flags = 0;
    time_t exptime = 1;
    int bytes = -1;
    uint64_t cas = 0;

    Reader r(begin,end);
    good = good && r.read(&flags) && r.read(&exptime) && r.read(&bytes);

    int rel_exptime = static_cast<int>(exptime);
    if(exptime > 60 * 60 * 24 * 30){
        rel_exptime = static_cast<int>(exptime - owner_->startTime());
        if(rel_exptime < 1){
            rel_exptime = 1;
        }
    }else{
    }

    if(good && policy_ == Item::kCas){
        good = r.read(&cas);
    }
    if(!good){
        reply("CLIEN_ERROR bad command line format\r\n");
        return true;
   }

   if(bytes > 1024 * 1024){
       reply("SERVER_ERROR object too large for cache\r\n");
       needle_->resetKey(key);
       owner_->deleteItem(needle_);
       bytesToDiscard_ = bytes + 2;
       state_ = kDiscardValue;
       return false;
   }else{
       currItem_ = Item::makeItem(key,flags,rel_exptime,bytes + 2,cas);
       state_ = kReceiveValue;
       return false;
   }
}
void Session::doDelete(Session::Tokenizer::iterator& begin,
                       Session::Tokenizer::iterator end){
    assert(command_ == "delete");
    JKYi::net::StringPiece key = *begin;
    bool good = (key.size() <= kLongestKeySize);
    ++begin;
    if(!good){
        reply("CLIENT_ERROR bad command line format\r\n");
    }else if(begin != end && *begin != "0"){
        reply("CLIENT_ERROR bad command line format, Usage: delete <key> [noreply]\r\n");
    }else{
        needle_->resetKey(key);
        if(owner_->deleteItem(needle_)){
            reply("DELETED\r\n");
        }else{
            reply("NOT_FOUND\r\n");
        }
    }
}

