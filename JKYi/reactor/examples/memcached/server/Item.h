#ifndef _JKYI_NET_MEMCACHED_ITEM_H_
#define _JKYI_NET_MEMCACHED_ITEM_H_

#include"JKYi/atomic.h"
#include"JKYi/reactor/StringPiece.h"
#include"JKYi/Types.h"
#include"JKYi/noncopyable.h"
#include"JKYi/reactor/Buffer.h"

#include<memory>

class Item;
typedef std::shared_ptr<Item> ItemPtr;
typedef std::shared_ptr<const Item> ConstItemPtr;

class Item : JKYi::Noncopyable{
public:
    //具体进行的操作
    enum UpdatePolicy{
        kInvalid,
        kSet,
        kAdd,
        kReplace,
        kAppend,
        kPrepend,
        kCas,
    };

    static ItemPtr makeItem(JKYi::net::StringPiece key,
                             uint32_t flags,
                             int exptime,
                             int valuelen,
                             uint64_t cas){
        return std::make_shared<Item>(key,flags,exptime,valuelen,cas);
    }

    Item(JKYi::net::StringPiece key,uint32_t flags,int exptime,int valuelen,uint64_t cas);
    ~Item(){
        ::free(data_);
    }

    JKYi::net::StringPiece key()const{
        return JKYi::net::StringPiece(data_,keylen_);
    }
    uint32_t flags()const{
        return flags_;
    }
    int rel_exptime()const{
        return rel_exptime_;
    }
    const char* value()const{
        return data_ + keylen_;
    }
    size_t valueLength()const{
        return valuelen_;
    }
    uint64_t cas()const{
        return cas_;
    }
    size_t hash()const{
        return hash_;
    }

    void setCas(uint64_t cas){
        this->cas_ = cas;
    }
    size_t neededBytes()const{
        return totalLen() - receivedBytes_;
    }


    void append(const char* data,size_t len);
    bool endsWithCRLF()const{
        return receivedBytes_ == totalLen()
               && data_[totalLen() - 2] == '\r'
               && data_[totalLen() - 1] == '\n';
    }

    void output(JKYi::net::Buffer* out,bool needCas = false)const;
    void resetKey(JKYi::net::StringPiece k);
private:
    int totalLen()const{
        return keylen_ + valuelen_;
    }

    int                 keylen_;          //key的长度
    const uint32_t      flags_;           //用来携带一些客户端自定义的数据
    const int           rel_exptime_;     //过期时间
    const int           valuelen_;        //value 的长度
    int                 receivedBytes_;   
    uint64_t            cas_;             //实现乐观锁,保证数据的一致性,通过该成员来验证数据的版本号
    size_t              hash_;            //该节点对应的哈希值
    char*               data_;            //元素存储的内存
};

#endif
