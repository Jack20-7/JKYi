#ifndef _JKYI_NET_MEMCACHE_SESSION_H_
#define _JKYI_NET_MEMCACHE_SESSION_H_

#include"JKYi/reactor/examples/memcached/server/Item.h"
#include"JKYi/reactor/Logging.h"
#include"JKYi/reactor/TcpConnection.h"

#include"/usr/local/include/boost/tokenizer.hpp"

class MemcacheServer;

//每一个连接的客户端对应一个session
class Session : public std::enable_shared_from_this<Session>,
                       JKYi::Noncopyable{
public:
    Session(MemcacheServer* owner,const JKYi::net::TcpConnectionPtr& conn)
        :owner_(owner),
         conn_(conn),
         state_(kNewCommand),
         protocol_(kAscii),
         noreply_(false),
         policy_(Item::kInvalid),
         bytesToDiscard_(0),
         needle_(Item::makeItem(kLongestKey,0,0,2,0)),
         bytesRead_(0),
         requestsProcessed_(0){

         using std::placeholders::_1;
         using std::placeholders::_2;
         using std::placeholders::_3;

         conn_->setMessageCallback(
                 std::bind(&Session::onMessage,this,_1,_2,_3));
    }
    
    ~Session(){
        LOG_INFO << "requests processed: " <<requestsProcessed_
                 << " input buffer size: " << conn_->inputBuffer()->internalCapacity()
                 << " output buffer size: " << conn_->outputBuffer()->internalCapacity();
    }
private:
   enum State{
       kNewCommand,
       kReceiveValue,
       kDiscardValue,
   };
   enum Protocol{
       kAscii,
       kBinary,
       kAuto,
   };

   void onMessage(const JKYi::net::TcpConnectionPtr& conn,
                   JKYi::net::Buffer* buf,
                   JKYi::net::Timestamp receiveTime);
   //void onWriteComplete(const JKYi::net::TcpConnectionPtr& conn);
   void receiveValue(JKYi::net::Buffer* buf);
   void discardValue(JKYi::net::Buffer* buf);

   bool processRequest(JKYi::net::StringPiece request);
   void resetRequest();
   void reply(JKYi::net::StringPiece msg);

   //自定义的分词规则，只要里面有reset和operator() 就ok了
   struct SpaceSeparator{
       void reset(){}
       template<class InputIterator,class Token>
        bool operator() (InputIterator& next,InputIterator end,Token& tok);
   };

   //第一个模板参数就是分词规则
   //第二个模板参数就是字符序列的迭代器类型
   //第三个模板参数就是保存分词结果的类型
   typedef boost::tokenizer<SpaceSeparator,const char*,JKYi::net::StringPiece> Tokenizer;

   struct Reader;
   bool doUpdate(Tokenizer::iterator& begin,Tokenizer::iterator end);
   void doDelete(Tokenizer::iterator& begin,Tokenizer::iterator end);

   MemcacheServer* owner_ ;                    //属于哪一个服务器
   JKYi::net::TcpConnectionPtr conn_;          //对应的是一条TCP连接
   State state_;
   Protocol protocol_;

   //current request
   std::string command_;                    
   bool noreply_;                             //当前正在处理的这一条命令是否需要响应
   Item::UpdatePolicy policy_;
   ItemPtr  currItem_;                        
   size_t bytesToDiscard_;

   //cached
   ItemPtr needle_;                           //在get或delete时会使用到
   JKYi::net::Buffer outputBuf_;

   //per session stats
   size_t bytesRead_;
   size_t requestsProcessed_;              //该连接上处理过的请求数

   static std::string kLongestKey;
};

typedef std::shared_ptr<Session> SessionPtr;

#endif
