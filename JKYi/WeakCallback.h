#ifndef _JKYI_WEAKCALLBACL_H_
#define _JKYI_WEAKCALLBACK_H_

#include<functional>
#include<memory>

namespace JKYi{

//弱回调技术
//class 表示的是调用的是那一个类中的成员函数
//ARGS表示的是调用的该成员函数的参数
template<typename CLASS,typename ... ARGS>
class WeakCallback{
public:
    WeakCallback(const std::weak_ptr<CLASS>& object,
                   const std::function<void (CLASS*,ARGS...)>& function)
        :object_(object),
         function_(function){
     }
    void operator() (ARGS&&... args)const{
        std::shared_ptr<CLASS> ptr(object_.lock());
        if(ptr){
            function_(ptr.get(),std::forward<ARGS>(args)...);
        }
    }
private:
    std::weak_ptr<CLASS> object_;
    std::function<void (CLASS*,ARGS...)> function_;
};
template<typename CLASS,typename ...ARGS>
WeakCallback<CLASS,ARGS...> makeWeakCallback(const std::shared_ptr<CLASS>& object,void (CLASS::* function)(ARGS...)){
    return WeakCallback<CLASS,ARGS...>(object,function);
}
template<typename CLASS,typename ...ARGS>
WeakCallback<CLASS,ARGS...> makeWeakCallback(const std::shared_ptr<CLASS>& object,void (CLASS::* function)(ARGS...)const){
    return WeakCallback<CLASS,ARGS...>(object,function);
}

}

#endif
