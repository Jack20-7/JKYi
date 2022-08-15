#ifndef _JKYI_HASH_UTIL_H_
#define _JKYI_HASH_UTIL_H_

#include<stdint.h>
#include<string>
#include<vector>

namespace JKYi{

std::string base64decode(const std::string& src);
std::string base64encode(const std::string& data);
std::string base64encode(const void * data,size_t len);

std::string sha1sum(const std::string& data);
std::string sha1sum(const void * data,size_t len);

std::string random_string(size_t len,
                       const std::string& chars = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ");

}

#endif
