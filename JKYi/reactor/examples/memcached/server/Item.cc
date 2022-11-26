#include"JKYi/reactor/examples/memcached/server/Item.h"
#include"JKYi/reactor/LogStream.h"

#include"/usr/local/include/boost/functional/hash/hash.hpp"

#include<stdio.h>
#include<string.h>

using namespace JKYi;
using namespace JKYi::net;

Item::Item(StringPiece key,
           uint32_t flags,
           int exptime,
           int valuelen,
           uint64_t cas)
    :keylen_(key.size()),
     flags_(flags),
     rel_exptime_(exptime),
     valuelen_(valuelen),
     receivedBytes_(0),
     cas_(cas),
     hash_(boost::hash_range(key.begin(),key.end())),
     data_(static_cast<char*>(::malloc(totalLen()))){

     assert(valuelen_ >= 2);
     assert(receivedBytes_ < totalLen());
     append(key.data(),keylen_);
}

void Item::append(const char* data,size_t len){
    assert(len <= neededBytes());
    memcpy(data_ + receivedBytes_,data,len);
    receivedBytes_ += static_cast<int>(len);
    assert(receivedBytes_ <= totalLen());
}

void Item::output(Buffer* out,bool needCas)const{
    out->append("VALUE ");
    out->append(data_,keylen_);
    LogStream buf;
    buf << ' ' << flags_ << ' ' << valuelen_ - 2;
    if(needCas){
        buf << ' ' << cas_;
    }
    buf << "\r\n";
    out->append(buf.buffer().data(),buf.buffer().length());
    out->append(value(),valuelen_);
}

void Item::resetKey(StringPiece k){
    assert(k.size() <= 250);
    keylen_ = k.size();
    receivedBytes_ = 0;
    append(k.data(),keylen_);
    hash_ = boost::hash_range(k.begin(),k.end());
}




