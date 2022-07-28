#ifndef _JKYI_ADDRESS_H_
#define _JKYI_ADDRESS_H_

#include<memory>
#include<string>
#include<sys/types.h>
#include<sys/socket.h>
#include<sys/un.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<iostream>
#include<vector>
#include<map>

namespace JKYi{
//对socket地址进行封装
//     ------Address------
//		 -        -
//		-          -
//	   -            -
//	 IPAddress    UnixAddress
//	  -     -
//	 -       -
//	-         -
// IPv4Address   IPv6Address

class IPAddress;

class Address{
public:
	typedef std::shared_ptr<Address> ptr;

	//根据传入的addr返回对应的Address的智能指针，如果失败就返回nullptr
	static Address::ptr Create(const sockaddr*addr,socklen_t addrlen);

	static bool Lookup(std::vector<Address::ptr>&result,const std::string&host,int family=AF_INET,int type=0,int protocol=0);

	static Address::ptr LookupAny(const std::string&host,int family=AF_INET,int type=0,int protocol=0);

	static std::shared_ptr<IPAddress> LookupAnyIPAddress(const std::string&host,int family=AF_INET,int type=0,int protocol=0);

	//返回当前机器上所有网卡的<网卡名、网卡地址、网卡的子网掩码>
	static bool GetInterfaceAddress(std::multimap<std::string,std::pair<Address::ptr,uint32_t>>&result,int family=AF_INET);

	//返回指定网卡的地址和子网掩码
	static bool GetInterfaceAddress(std::vector<std::pair<Address::ptr,uint32_t>>&result,const std::string&iface,int family=AF_INET);

	//该类会被作为抽象类被其他的类所继承
	virtual ~Address(){}

	int getFamily()const;

	//留给子类实现的接口
	virtual const sockaddr* getAddr()const=0;

	virtual sockaddr* getAddr()=0;

	virtual socklen_t getAddrLen()const=0;

	virtual std::ostream& insert(std::ostream& os)const =0;
	//
	std::string toString()const;

	//下面是对一些常见的运算符进行重载，因为地址类可能需要被转入STL容器
   bool operator< (const Address&rhv)const;

   bool operator== (const Address&rhv)const;

   bool operator!= (const Address&rhv)const;

};

//IP地址的地址
class IPAddress:public Address{
public:
	typedef std::shared_ptr<IPAddress> ptr;

    //根据IP和port返回对应的IPAddress
	static IPAddress::ptr Create(const char*address,uint16_t port=0);

    //根据子网掩码的位数获取对应的广播地址
	virtual IPAddress::ptr broadcastAddress(uint32_t prefix_len)=0;
     
	//根据子网掩码的中1的个数返回IP对应的网段
	virtual IPAddress::ptr networkAddress(uint32_t prefix_len)=0;

	//根据子网掩码的位数返回对应的子网掩码
	virtual IPAddress::ptr subnetMaskAddress(uint32_t prefix_len)=0;

	virtual uint16_t getPort()const = 0;

	//设置端口号
	virtual void setPort(uint16_t v) = 0;

private:

};

//IPv4的地址
class IPv4Address:public IPAddress{
public:
	typedef std::shared_ptr<IPv4Address> ptr;
	//
	static IPv4Address::ptr Create(const char*addr,uint16_t port=0);

	IPv4Address(const sockaddr_in&addr);

	IPv4Address(uint32_t address=INADDR_ANY,uint16_t port=0);
    
	const sockaddr* getAddr()const override;

	sockaddr* getAddr()override;

	socklen_t getAddrLen()const override;

	std::ostream& insert(std::ostream&os)const override;

	IPAddress::ptr broadcastAddress(uint32_t prefix_len)override;
	IPAddress::ptr networkAddress(uint32_t prefix_len)override;
	IPAddress::ptr subnetMaskAddress(uint32_t prefix_len)override;

	uint16_t getPort()const override;
	void setPort(uint16_t v)override;

private:
	sockaddr_in m_addr;
};

//IPv6的地址
//IPv6的地址是128位，然后每16位一组
class IPv6Address:public IPAddress{
public:
	typedef std::shared_ptr<IPv6Address> ptr;

	static IPv6Address::ptr Create(const char*address,uint16_t port=0);

	IPv6Address();

	IPv6Address(const sockaddr_in6&address);

	IPv6Address(const uint8_t address[16],uint16_t port=0);

	const sockaddr* getAddr()const override;

	sockaddr* getAddr()override;

	socklen_t getAddrLen()const override;

	std::ostream& insert(std::ostream&os)const override;

	IPAddress::ptr broadcastAddress(uint32_t prefix_len)override;
	IPAddress::ptr networkAddress(uint32_t prefix_len)override;
	IPAddress::ptr subnetMaskAddress(uint32_t predix_len)override;
    uint16_t getPort()const override;
	void setPort(uint16_t v)override;
private:
	sockaddr_in6 m_addr;
};
//unix socket的地址
//
class UnixAddress:public Address{
public:
	typedef std::shared_ptr<UnixAddress> ptr;

	UnixAddress();

	//根据路径来构造UnixSocket的地址
	UnixAddress(const std::string& path);

	const sockaddr* getAddr()const override;
	sockaddr* getAddr()override;
	socklen_t getAddrLen()const override;
	void setAddrLen(uint32_t v);
	std::string getPath()const;
	std::ostream& insert(std::ostream&os)const override;
private:
	sockaddr_un m_addr;
	socklen_t m_length;
};

//未知地址
class UnknowAddress:public Address{
public:
	typedef std::shared_ptr<UnknowAddress> ptr;

	UnknowAddress(int family);
	UnknowAddress(const sockaddr&addr);
	const sockaddr* getAddr()const override;
	sockaddr* getAddr()override;
    socklen_t getAddrLen()const override;
	std::ostream& insert(std::ostream&os)const override;
private:
	sockaddr m_addr;
};

std::ostream& operator<< (std::ostream&os,const Address&rhv);



}

#endif
