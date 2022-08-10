#include"address.h"
#include"log.h"
#include"endian.h"

#include<sstream>
#include<netdb.h>
#include<ifaddrs.h>
#include<stddef.h>

namespace JKYi{
    
static JKYi::Logger::ptr g_logger=JKYI_LOG_NAME("system");

//根据传入的位数创建出对应的子网掩码
//bits表示的是创建出的子网掩码中1的个数
//返回的值中的0所组成的一段表示的就是网段，1所组成的那一段区域表示的是主机部分
template<class T>
static T CreateMask(uint32_t bits){
    return (1<<(sizeof(T)*8-bits))-1;
}

//该函数的作用就是返回value二进制位中1的个数
template<class T>
static uint32_t CountBytes(T value){
    uint32_t result=0;
    for(;value;result++){
        value &= (value-1);
    }
    return result;
}
//根据传入的域名返回对应的Address::ptr类型的数组
bool Address::Lookup(std::vector<Address::ptr>&result,const std::string&host,int family,int type,int protocol){

   addrinfo hints,*results,*next;    
   //对里面的信息进行初始化
   hints.ai_flags=0;
   hints.ai_family=family;
   hints.ai_socktype=type;
   hints.ai_protocol=protocol;
   hints.ai_addrlen=0;
   hints.ai_canonname=NULL;
   hints.ai_addr=NULL;
   hints.ai_next=NULL;
   //
   //存储IP地址
   std::string node;
   //存储端口号
   const char *service=NULL;
   
   //首先判断是否是IPv6的地址
   if(!host.empty() && host[0]=='['){
       //
       const char *endipv6=(const char*)memchr(host.c_str(),']',host.size()-1);
       if(endipv6){
         if(*(endipv6+1) == ':'){
             //
             service=endipv6+1;
         }
         node=host.substr(1,endipv6-host.c_str()-1);
       }
   }

   //检测如果没有取到IPv6的值，那在检查当前是否是IPv4的情况
   if(node.empty()){
       service=(const char*)memchr(host.c_str(),':',host.size());
       if(service){
          if(!memchr(service+1,':',host.c_str()+host.size()-service-1)){
              node=host.substr(0,service-host.c_str());
              service++;
          }
       }
   }

   if(node.empty()){
       node=host;
   }
   //获取信息
   int error=getaddrinfo(node.c_str(),service,&hints,&results);
   if(error){
       JKYI_LOG_ERROR(g_logger)<<"Address::Lookup getaddrinfo("<<host<<","
                               <<family<<","<<type<<") errno"<<" errnostr="
                               <<strerror(errno);
       return false;
   }
  
   next=results;

   while(next){
       result.push_back(Create(next->ai_addr,(socklen_t)next->ai_addrlen));
       next=next->ai_next;
   }

   freeaddrinfo(results);
   return !result.empty();
}
//
Address::ptr Address::LookupAny(const std::string&host,int family,int type,int protocol){
    std::vector<Address::ptr>result;

    if(Lookup(result,host,family,type,protocol)){
        return result[0];
    }
    return nullptr;
}

IPAddress::ptr Address::LookupAnyIPAddress(const std::string&host,int family,int type,int protocol){
    std::vector<Address::ptr>result;

    if(Lookup(result,host,family,type,protocol)){
       for(auto&i : result){
           IPAddress::ptr v=std::dynamic_pointer_cast<IPAddress>(i);
           if(v){
               return v;
           }
       }
    }
    return nullptr;
}

//返回当前计算机中所有网卡的信息
//网卡的名称 网卡对应的地址信息 网卡子网掩码的信息
bool Address::GetInterfaceAddress(std::multimap<std::string,std::pair<Address::ptr,uint32_t>>&result,int family){
    struct ifaddrs*next,*results;
    if(getifaddrs(&results)!=0){
       JKYI_LOG_ERROR(g_logger)<<"Address::GetInterfaceAddress getifaddress"<< " errno="<<errno<<" errnostr="<<strerror(errno);
       return false;
    }
    //
    try{
        for(next=results;next;next=next->ifa_next){
           Address::ptr addr;
           //先默认是无穷大,存储的是子网掩码中1的个数
           uint32_t prefix_len= ~0u;
           if(family !=AF_UNSPEC && family!=next->ifa_addr->sa_family){
               continue;
           }
           switch(next->ifa_addr->sa_family){
               case AF_INET:
                   {
                       addr=Create(next->ifa_addr,sizeof(sockaddr_in));
                       uint32_t netmask=((sockaddr_in*)next->ifa_netmask)->sin_addr.s_addr;
                       prefix_len=CountBytes(netmask);
                   }
                   break;
               case AF_INET6:
                   {
                       addr=Create(next->ifa_addr,sizeof(sockaddr_in6));
                       in6_addr&netmask=((sockaddr_in6*)next->ifa_netmask)->sin6_addr;
                       prefix_len=0;
                       for(int i=0;i<16;++i){
                           prefix_len+=CountBytes(netmask.s6_addr[i]);
                       }
                   }
                   break;
                default:
                   break;
           }
           if(addr){
               result.insert(std::make_pair(next->ifa_name,std::make_pair(addr,prefix_len)));
           }

        }

    }catch(...){
        JKYI_LOG_ERROR(g_logger)<<"Address::GetInterfaceAddress exception";
        freeifaddrs(results);
        return false;
    }

    freeifaddrs(results);
    return !result.empty();
}
//返回指定网卡的信息
bool Address::GetInterfaceAddress(std::vector<std::pair<Address::ptr,uint32_t>>&result,const std::string&iface,int family){
    //如果用户没有显式的指定网卡的名称或者名称为*的话，那么就根据family返回对应的默认地址信息
   if(iface.empty() || iface =="*"){
       if(family == AF_INET || family == AF_UNSPEC){
           result.push_back(std::make_pair(Address::ptr(new IPv4Address()),0u));
       }
       if(family == AF_INET6 || family == AF_UNSPEC){
           result.push_back(std::make_pair(Address::ptr(new IPv6Address()),0u));
       }
       return true;
   }
   //
   std::multimap<std::string,std::pair<Address::ptr,uint32_t>> results;
   if(!GetInterfaceAddress(results,family)){
       return false;
   }

   auto its=results.equal_range(iface);
   for(;its.first != its.second;its.first++){
       result.push_back(its.first->second);
   }
   return !result.empty();
}


int Address::getFamily()const{
    return getAddr()->sa_family;
}

std::string Address::toString()const {
    std::stringstream ss;
    insert(ss);
    return ss.str();
}

Address::ptr Address::Create(const sockaddr*addr,socklen_t addrlen){
    if(addr == nullptr){
        return nullptr;
    }
    Address::ptr result;
    switch(addr->sa_family){
        case AF_INET:
            result.reset(new IPv4Address(*(const sockaddr_in*)addr));
            break;
        case AF_INET6:
            result.reset(new IPv6Address(*(const sockaddr_in6*)addr));
            break;
        default:
            result.reset(new UnknowAddress(*addr));
            break;
    }
    return result;
}
bool Address::operator< (const Address&rhv)const {
    socklen_t minlen=std::min(getAddrLen(),rhv.getAddrLen()); 
    int result=memcmp(getAddr(),rhv.getAddr(),minlen);

    if(result<0){
       return true;
    }
    else if(result>0){
       return false;
    }else if(getAddrLen()<rhv.getAddrLen()){
       return true;
    }
    return false;
}
//

bool Address::operator== (const Address&rhv)const{
    return getAddrLen()==rhv.getAddrLen() && memcmp(getAddr(),rhv.getAddr(),getAddrLen())==0;
}

bool Address::operator!= (const Address&rhv)const{
    return !(*this == rhv);
}


//
IPAddress::ptr IPAddress::Create(const char * address,uint16_t port){
    addrinfo hints,*results;
    memset(&hints,0,sizeof(hints));

    //如果是该参数的话，表示不接受域名的传入
    hints.ai_flags=AI_NUMERICHOST;
    hints.ai_family=AF_UNSPEC;

    int error=getaddrinfo(address,NULL,&hints,&results);
    if(error){
        JKYI_LOG_ERROR(g_logger)<<"IPAddress::Create ("<<address<<","<<port
                                <<") error="<<error<<" errorstr="<<strerror(error);
        return nullptr;
    }
    //
    try{
        IPAddress::ptr result=std::dynamic_pointer_cast<IPAddress>(Address::Create(results->ai_addr,(socklen_t)results->ai_addrlen));
        if(result){
            result->setPort(port);
        }
        freeaddrinfo(results);
        return result;

    }catch(...){
        freeaddrinfo(results);
        return nullptr;
    }
}

//
IPv4Address::ptr IPv4Address::Create(const char *addr,uint16_t port){
        
    IPv4Address::ptr result(new IPv4Address());
    result->m_addr.sin_port=toNetEndian(port);
    int ret=inet_pton(AF_INET,addr,&result->m_addr.sin_addr);
    if(ret<=0){
        JKYI_LOG_ERROR(g_logger)<<"IPv4Address::Create(" << addr << ", "
                                << port << ") ret=" << ret << " errno=" 
                                << errno<< " errstr=" << strerror(errno);
        return nullptr;
    }
    return result;
}

IPv4Address::IPv4Address(uint32_t address,uint16_t port){
   memset(&m_addr,0,sizeof(m_addr));
   m_addr.sin_family=AF_INET;
   m_addr.sin_addr.s_addr=toNetEndian(address);
   m_addr.sin_port=toNetEndian(port);
}

IPv4Address::IPv4Address(const sockaddr_in&addr){
    m_addr=addr;
}

sockaddr* IPv4Address::getAddr(){
    return (sockaddr*)&m_addr;
}

const sockaddr* IPv4Address::getAddr()const {
    return (sockaddr*)&m_addr;
}

socklen_t IPv4Address::getAddrLen()const {
    return sizeof(m_addr);
}

std::ostream& IPv4Address::insert(std::ostream &os)const {
    uint32_t addr=toNetEndian(m_addr.sin_addr.s_addr);
   // uint32_t addr=m_addr.sin_addr.s_addr;
    os<< ((addr>>24) & 0xff)<<"."
      << ((addr>>16) & 0xff)<<"."
      << ((addr>>8) & 0xff)<<"."
      << (addr & 0xff);
    os<<":"<<toNetEndian(m_addr.sin_port);
    return os;
}
//传入的参数表示的是子网掩码中1的个数
IPAddress::ptr IPv4Address::broadcastAddress(uint32_t prefix_len){
    if(prefix_len>32){
        return nullptr;
    }
    sockaddr_in baddr(m_addr);
    baddr.sin_addr.s_addr|= toNetEndian(CreateMask<uint32_t>(prefix_len));
    return IPv4Address::ptr(new IPv4Address(baddr));
}

IPAddress::ptr IPv4Address::networkAddress(uint32_t prefix_len){
    if(prefix_len>32){
        return nullptr;
    }
    //
    sockaddr_in baddr(m_addr);
    baddr.sin_addr.s_addr &= ~toNetEndian(CreateMask<uint32_t>(prefix_len));
    return IPv4Address::ptr (new IPv4Address(baddr));
}

//
IPAddress::ptr IPv4Address::subnetMaskAddress(uint32_t prefix_len){
   if(prefix_len >32){
       return nullptr;
   }
   //
   sockaddr_in subnet;
   memset(&subnet,0,sizeof(subnet));
   subnet.sin_family=AF_INET;
   subnet.sin_addr.s_addr= ~toNetEndian(CreateMask<uint32_t>(prefix_len));
   return IPv4Address::ptr (new IPv4Address(subnet));
}

uint16_t IPv4Address::getPort()const{
    return toNetEndian(m_addr.sin_port);
}

void IPv4Address::setPort(uint16_t v){
    m_addr.sin_port=toNetEndian(v);
    return ;
}
//

IPv6Address::ptr IPv6Address::Create(const char*address,uint16_t port){

     IPv6Address::ptr rt(new IPv6Address());
     rt->m_addr.sin6_port=toNetEndian(port);
     int result=inet_pton(AF_INET6,address,&rt->m_addr.sin6_addr);
     if(result<=0){
         JKYI_LOG_ERROR(g_logger)<<"IPv6Address::Create("<<address<<","
                                 <<port<<")"<<" errno="<<errno
                                 <<" strerrno="<<strerror(errno);
         return nullptr;
     }
     return rt;
}

IPv6Address::IPv6Address(){
    memset(&m_addr,0,sizeof(m_addr));
    m_addr.sin6_family=AF_INET6;
}

IPv6Address::IPv6Address(const sockaddr_in6&addr){
    m_addr=addr;
}

IPv6Address::IPv6Address(const uint8_t address[16],uint16_t port){
    memset(&m_addr,0,sizeof(m_addr));
    m_addr.sin6_family=AF_INET6;
    m_addr.sin6_port=toNetEndian(port);
    memcpy(&m_addr.sin6_addr.s6_addr,address,16);
}

sockaddr* IPv6Address::getAddr(){
    return (sockaddr*)&m_addr;
}

const sockaddr* IPv6Address::getAddr()const{
    return (sockaddr*)&m_addr;
}

socklen_t IPv6Address::getAddrLen()const{
    return sizeof(m_addr);
}

std::ostream& IPv6Address::insert(std::ostream&os)const{
    os<<"[";
    uint16_t *addr=(uint16_t*)m_addr.sin6_addr.s6_addr;
    bool used_zero=false;
    for(size_t i=0;i<8;++i){
        if(addr[i]==0 && !used_zero){
            continue;
        }
        if(i && addr[i-1]==0 && !used_zero){
           os<<":";
           used_zero=true;
        }
        if(i){
            os<<":";
        }
        os<<std::hex<<(int)toNetEndian(addr[i])<<std::dec;
    }
    if(!used_zero &&addr[7] ==0 ){
       os<<"::";
    }
    os<<"]:"<<toNetEndian(m_addr.sin6_port);
    return os;
}

IPAddress::ptr IPv6Address::broadcastAddress(uint32_t prefix_len){
     sockaddr_in6 baddr(m_addr);
     baddr.sin6_addr.s6_addr[prefix_len/8] |= CreateMask<uint8_t>(prefix_len%8);
     for(int i=prefix_len/8+1;i<16;++i){
        baddr.sin6_addr.s6_addr[i] = 0xff; 
     }
     return IPv6Address::ptr (new IPv6Address(baddr));
}

IPAddress::ptr IPv6Address::networkAddress(uint32_t prefix_len){
    sockaddr_in6 baddr(m_addr); 
    baddr.sin6_addr.s6_addr[prefix_len/8] &= ~CreateMask<uint8_t>(prefix_len%8);
    for(int i=prefix_len/8+1;i<16;++i){
        baddr.sin6_addr.s6_addr[i]=0x00;
    }
    return IPv6Address::ptr (new IPv6Address(baddr));
}

IPAddress::ptr IPv6Address::subnetMaskAddress(uint32_t prefix_len){
    sockaddr_in6 subnet;
    memset(&subnet,0,sizeof(subnet));
    subnet.sin6_family=AF_INET6;
    subnet.sin6_addr.s6_addr[prefix_len/8]= ~CreateMask<uint8_t>(prefix_len%8);
    for(uint32_t i=prefix_len/8;i>=0;--i){
        subnet.sin6_addr.s6_addr[i]=0xff;
    }
    return IPv6Address::ptr (new IPv6Address(subnet));

}

uint16_t IPv6Address::getPort()const{
    return toNetEndian(m_addr.sin6_port);
}

void IPv6Address::setPort(uint16_t v){
    m_addr.sin6_port=toNetEndian(v);
}
//
static const size_t MAX_PATH_LEN=sizeof(((sockaddr_un*)0)->sun_path)-1;

UnixAddress::UnixAddress(){
    memset(&m_addr,0,sizeof(m_addr));
    m_addr.sun_family=AF_UNIX;
    m_length=offsetof(sockaddr_un,sun_path)+MAX_PATH_LEN;
}

UnixAddress::UnixAddress(const std::string&path){
    memset(&m_addr,0,sizeof(m_addr));
    m_addr.sun_family=AF_UNIX;
    m_length=path.size()+1;

    if(!path.empty() && path[0]=='\0'){
        --m_length;
    }
    if(m_length>sizeof(m_addr.sun_path)){
            throw std::logic_error("path too long");
    }
    memcpy(m_addr.sun_path,path.c_str(),m_length);
    m_length+=offsetof(sockaddr_un,sun_path);
}

void UnixAddress::setAddrLen(uint32_t len){
   m_length=len;
}

sockaddr* UnixAddress::getAddr(){
   return (sockaddr*)&m_addr;
}

const sockaddr* UnixAddress::getAddr()const {
   return (sockaddr*)&m_addr;
}

socklen_t UnixAddress::getAddrLen()const{
   return m_length;
}

std::string UnixAddress::getPath()const{
    std::stringstream ss;
    if(m_length > offsetof(sockaddr_un,sun_path) 
            && m_addr.sun_path[0] == '\0'){
        ss<<"\\"<<std::string(m_addr.sun_path+1,m_length-offsetof(sockaddr_un,
                               sun_path)-1);
    }else{
        ss<<m_addr.sun_path;
    }
    return ss.str();
}

std::ostream& UnixAddress::insert(std::ostream&os)const {
    if(m_length > offsetof(sockaddr_un,sun_path) &&
            m_addr.sun_path[0] == '\0'){
        return os<<"\\0"<<std::string(m_addr.sun_path+1,m_length-
                                 offsetof(sockaddr_un,sun_path)-1);
    }
    return os<<m_addr.sun_path;
}
//
UnknowAddress::UnknowAddress(const sockaddr&addr){
    m_addr=addr;
}

UnknowAddress::UnknowAddress(int family){
    memset(&m_addr,0,sizeof(m_addr));
    m_addr.sa_family=family;
}

sockaddr* UnknowAddress::getAddr(){
    return &m_addr;
}

const sockaddr* UnknowAddress::getAddr()const {
    return &m_addr;
}

socklen_t UnknowAddress::getAddrLen()const {
    return sizeof(m_addr);
}

std::ostream& UnknowAddress::insert(std::ostream&os)const{
    os<<"[UnkowAddress family ="<<m_addr.sa_family<<"]";
    return os;
}

std::ostream& operator<< (std::ostream&os,const Address&addr){
    return addr.insert(os);
}

}
