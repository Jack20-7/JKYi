#include"query.pb.h"
#include"boost/noncopyable.hpp"

#include<functional>
#include<memory>
#include<iostream>
#include<map>

class Callback : public boost::noncopyable{
public:
    virtual ~Callback() {}
    virtual void onMessage(google::protobuf::Message* message) const = 0;
};

template<typename T>
class CallbackT : public Callback{
public:
    typedef std::function<void (T*)> ProtobufMessageCallback;
    CallbackT(const ProtobufMessageCallback& callback)
        :callback_(callback){}
    virtual void onMessage(google::protobuf::Message* message)const override{
        T* t = dynamic_cast<T*>(message);
        callback_(t);
    }
private:
    ProtobufMessageCallback callback_;
};

void discardProtobufMessage(google::protobuf::Message* message){
    std::cout << " Discarding " << message->GetTypeName() << std::endl;
}

class ProtobufDispatcher{
public:
    ProtobufDispatcher()
        :defaultCallback_(discardProtobufMessage){}

    void onMessage(google::protobuf::Message* message){
        CallbackMap::const_iterator it = callbacks_.find(message->GetDescriptor());
        if(it != callbacks_.end()){
            it->second->onMessage(message);
        }else{
            defaultCallback_(message);
        }
    }
    template<typename T>
    void registerMessageCallback(const typename CallbackT<T>::ProtobufMessageCallback& callback){
        std::shared_ptr<CallbackT<T> > pd(new CallbackT<T>(callback));
        callbacks_[T::descriptor()] = pd;
    }
private:
    typedef std::map<const google::protobuf::Descriptor*,std::shared_ptr<Callback> > CallbackMap;

    CallbackMap callbacks_;
    std::function<void (google::protobuf::Message* message)> defaultCallback_;
};

void onQuery(test::Query* query){
    std::cout << " onQuery: " << query->GetTypeName() << std::endl;
}
void onAnswer(test::Answer* answer){
    std::cout << " onAnswer: " << answer->GetTypeName() << std::endl;
}


int main(int argc,char ** argv){
    ProtobufDispatcher dispatcher;
    dispatcher.registerMessageCallback<test::Query>(onQuery);
    dispatcher.registerMessageCallback<test::Answer>(onAnswer);

    test::Query q;
    test::Answer a;
    test::Empty e;

    dispatcher.onMessage(&q);
    dispatcher.onMessage(&a);
    dispatcher.onMessage(&e);

    google::protobuf::ShutdownProtobufLibrary();
}


