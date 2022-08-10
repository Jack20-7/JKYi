#ifndef _JKYI_LIBRARY_H_
#define _JKYI_LIBRARY_H_

#include<memory>
#include"module.h"

namespace JKYi{

//该类负责用来对模块进行加载
class Library{
public:
    static Module::ptr GetModule(const std::string& path);
};
}

#endif
