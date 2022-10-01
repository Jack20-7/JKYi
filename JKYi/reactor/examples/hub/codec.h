#ifndef _JKYI_EXAMPLES_HUB_H_
#define _JKYI_EXAMPLES_HUB_H_

#include"JKYi/Types.h"
#include"JKYi/reactor/Buffer.h"

#include<string>

namespace pubsub{

enum ParseResult{
    kError = 0,
    kSuccess = 1,
    kContinue = 2,
};

ParseResult parseMessage(JKYi::net::Buffer* buf,
                          std::string * cmd,
                          std::string * topic,
                          std::string * content);
}//namespace pubsub

#endif
