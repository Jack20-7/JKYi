#include"../JKYi/JKYi.h"

static JKYi::Logger::ptr g_logger=JKYI_LOG_ROOT();

void test(){
    std::vector<JKYi::Address::ptr>results;
    JKYI_LOG_INFO(g_logger)<<"begin";

    bool ret=JKYi::Address::Lookup(results,"www.baidu.com:http");

    JKYI_LOG_INFO(g_logger)<<"end";
    if(!ret){
        JKYI_LOG_ERROR(g_logger)<<"lookup fail";
        return ;
    }
    for(size_t i=0;i<results.size();++i){
        JKYI_LOG_INFO(g_logger)<<i<<" - "<<results[i]->toString();
    }
    //
    auto addr=JKYi::Address::LookupAny("localhost:4080");
    if(addr){
        JKYI_LOG_INFO(g_logger)<<*addr;
    }else{
        JKYI_LOG_ERROR(g_logger)<<"error";
    }
    return ;
}

void test_iface(){
    std::multimap<std::string,std::pair<JKYi::Address::ptr,uint32_t>>results;

    bool ret=JKYi::Address::GetInterfaceAddress(results);
    if(!ret){
        JKYI_LOG_INFO(g_logger)<<"GetInterfaceAddress fail";
        return ;
    }
    for(auto&i : results){
        JKYI_LOG_INFO(g_logger)<<i.first<<" - "<<i.second.first->toString()<<" - "<<i.second.second;
    }
    return ;
}
void test_ipv4(){
    auto addr=JKYi::IPAddress::Create("127.0.0.1");
    JKYi::IPAddress::ptr network=addr->networkAddress(16);
    //JKYi::IPAddress::ptr subnetmask=addr->subnetMaskAddress(16);
    //auto addr=JKYi::IPAddress::Create("www.baidu.com");
    if(addr){
        JKYI_LOG_INFO(g_logger)<<addr->toString();
        JKYI_LOG_INFO(g_logger)<<network->toString();
        //JKYI_LOG_INFO(g_logger)<<subnetmask->toString();
    }
    return ;
}
int main(int argc,char**argv){
    test_ipv4();
    //test_iface();
    //test();
    return 0;
}

