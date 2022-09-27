#ifndef _JKYI_TIMESTAMP_H_
#define _JKYI_TIMESTAMP_H_

#include"boost/operators.hpp"
#include<stdint.h>

namespace JKYi{
namespace net{

//搞一个时间戳类

class Timestamp:public boost::equality_comparable<Timestamp>,
                public boost::less_than_comparable<Timestamp>{
public:

    static Timestamp now();
    static Timestamp invalid(){ return Timestamp(); }
    static Timestamp fromUnixTime(time_t t,int microseconds){ return Timestamp(static_cast<int64_t>(t) * kMicroSecondsPerSecond + microseconds); }
    static Timestamp fromUnixTime(time_t t){ return fromUnixTime(t,0); }
    //默认构造函数创建出的是一个无效的时间戳类
    Timestamp()
        :m_microSecondsSinceEpoch(0){
    }
    explicit Timestamp(int64_t microSecondsSinceEpochArg)
        :m_microSecondsSinceEpoch(microSecondsSinceEpochArg){
    }
    void swap(Timestamp& rhv){
        std::swap(m_microSecondsSinceEpoch,rhv.m_microSecondsSinceEpoch);
    }

    std::string toString()const;
    std::string toFormattedString(bool showMicroseconds = true)const;

    bool valid()const { return m_microSecondsSinceEpoch > 0; }

    int64_t getMicroSecondsSinceEpoch()const { return m_microSecondsSinceEpoch; }
    time_t getSecondsSinceEpoch()const{ 
        return static_cast<time_t>(m_microSecondsSinceEpoch / kMicroSecondsPerSecond);
    }

    //一秒 = 1000 * 1000 微秒
    static const int kMicroSecondsPerSecond = 1000 * 1000;
private:
    //从epoch到现在的微秒数 
    int64_t m_microSecondsSinceEpoch;
};

inline bool operator< (const Timestamp& lhv,const Timestamp& rhv){
   return lhv.getMicroSecondsSinceEpoch() < rhv.getMicroSecondsSinceEpoch();
}
inline bool operator== (const Timestamp& lhv,const Timestamp& rhv){
   return lhv.getMicroSecondsSinceEpoch() == rhv.getMicroSecondsSinceEpoch();
}
//返回两个时间戳之间相差的秒数
inline double timeDifference(const Timestamp& high,const Timestamp& low){
   int64_t diff = high.getMicroSecondsSinceEpoch() - low.getMicroSecondsSinceEpoch(); 
   return static_cast<double>(diff) / Timestamp::kMicroSecondsPerSecond;
}

inline Timestamp addTime(Timestamp timestamp,double seconds){
    int64_t delta = static_cast<int64_t>(seconds * Timestamp::kMicroSecondsPerSecond);
    return Timestamp(timestamp.getMicroSecondsSinceEpoch() + delta);
}

}
}

#endif
