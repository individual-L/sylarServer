#ifndef __ADDRESS_H__
#define __ADDRESS_H__

#include<sys/socket.h>
#include<sstream>
#include<iostream>
#include<string>
#include<memory>
#include<vector>
#include<map>
#include<unistd.h>
#include <netinet/in.h>
#include <sys/un.h>

namespace gaiya{

/*
// Structure describing a generic socket address. 
struct sockaddr
{
 uint16 sa_family;           // Common data: address family and length. 
 char sa_data[14];           //Address data.
};

//Structure describing an Internet socket address. 
struct sockaddr_in
{
 uint16 sin_family;          // Address family AF_INET 
 uint16 sin_port;            // Port number.   
 uint32 sin_addr.s_addr;     // Internet address.   
 unsigned char sin_zero[8];  // Pad to size of `struct sockaddr'.   
};
​
// Ditto, for IPv6.  
struct sockaddr_in6
{
 uint16 sin6_family;         // Address family AF_INET6
 uint16 sin6_port;           // Transport layer port # 
 uint32 sin6_flowinfo;       // IPv6 flow information 
 uint8  sin6_addr[16];       // IPv6 address 
 uint32 sin6_scope_id;       // IPv6 scope-id 
};
​
struct in6_addr
  {
    union
      {
    uint8_t __u6_addr8[16];
    uint16_t __u6_addr16[8];
    uint32_t __u6_addr32[4];
      } __in6_u;
 
*/

class IPAddress;

//网络地址
class Address{
   public:
      typedef std::shared_ptr<Address> ptr;

      virtual const sockaddr* getAddr() const = 0;

      virtual sockaddr* getAddr() = 0;

      virtual socklen_t getAddrSize() const = 0;

      virtual std::ostream& insert(std::ostream& os) const = 0;

      virtual ~Address(){}

      static Address::ptr Create(const sockaddr* sockaddr,const socklen_t sockLen);

      //返回host的所有符合条件的Address
      static bool Lookup(std::vector<Address::ptr>& addresses, const std::string& host,int family = AF_INET,int type = 0,int protocol = 0);

      //返回host的任意符合条件的Address
      static Address::ptr LookupAny(const std::string& hostName,int family = AF_INET,int type = 0,int protocol = 0);

      //返回host的任意符合条件的Address
      static std::shared_ptr<IPAddress> LookupAnyIPAddress(const std::string& hostName,int family = AF_INET,int type = 0,int protocol = 0);

      //
      static bool GetInterfaceAddress(std::multimap<std::string,std::pair<Address::ptr,uint32_t>>& results,int family = AF_INET);

      static bool GetInterfaceAddress(std::vector<std::pair<Address::ptr,uint32_t>>& res,const std::string& ifname,int family = AF_INET);

   public:
      int getFamily() const ;
      std::string toString() const ;

      bool operator==(const Address& addr) const ;

      bool operator<(const Address& addr) const ;

      bool operator!=(const Address& addr) const ;
};

//IP地址
class IPAddress : public Address{
   public:
      typedef std::shared_ptr<IPAddress> ptr;
      static IPAddress::ptr Create(const char* address, uint16_t port = 0);
   public:
      virtual IPAddress::ptr getBroadcastAddress(uint32_t prefix_len) = 0;
      virtual IPAddress::ptr getnetworkAddress(uint32_t prefix_len) = 0;
      virtual IPAddress::ptr getnetMask(uint32_t prefix_len) = 0;
      virtual uint16_t getPort() const = 0;
      virtual void setPort(uint16_t port) = 0;
};

class IPv4Address : public IPAddress{
   public:
      typedef std::shared_ptr<IPv4Address> ptr;

      IPv4Address(const sockaddr_in & addr);

      IPv4Address(uint32_t addr = INADDR_ANY,uint16_t port = 0);

   public:
      sockaddr* getAddr() override;

      const sockaddr* getAddr() const override;

      socklen_t getAddrSize() const override;

      std::ostream& insert(std::ostream& os) const override;

      IPAddress::ptr getBroadcastAddress(uint32_t prefix_len) override;

      IPAddress::ptr getnetworkAddress(uint32_t prefix_len) override;

      IPAddress::ptr getnetMask(uint32_t prefix_len) override;

      uint16_t getPort() const override;

      void setPort(uint16_t port) override;



   private:
      sockaddr_in m_addr;
};

class IPv6Address : public IPAddress{
   public:
      typedef std::shared_ptr<IPv6Address> ptr;

      IPv6Address();
      IPv6Address(const sockaddr_in6& addr);
      IPv6Address(const uint8_t addr[16],uint16_t port = 0);

      public:

         sockaddr* getAddr() override;

         const sockaddr* getAddr() const override;

         socklen_t getAddrSize() const override;

         std::ostream& insert(std::ostream& os) const override;

         IPAddress::ptr getBroadcastAddress(uint32_t prefix_len) override;

         IPAddress::ptr getnetworkAddress(uint32_t prefix_len) override;

         IPAddress::ptr getnetMask(uint32_t prefix_len) override;

         uint16_t getPort() const override;
         
         void setPort(uint16_t port) override;
   private:
      sockaddr_in6 m_addr;
};


//用于同一计算机中进程间通信
class UnixAddress : public Address {
   public:
      typedef std::shared_ptr<UnixAddress> ptr;

      UnixAddress();
      UnixAddress(const std::string& path);

      public:
         sockaddr* getAddr() override;

         const sockaddr* getAddr() const override;

         socklen_t getAddrSize() const override;

         std::ostream& insert(std::ostream& os) const override;

         void setAddrLen(const uint32_t len);

         std::string getPath() const ;
   private:
      sockaddr_un m_addr;
      socklen_t m_len;
};


class UknownAddress: public Address{
   public:
      typedef std::shared_ptr<UknownAddress> ptr;
      UknownAddress(){}
      UknownAddress(int family);
      UknownAddress(const sockaddr& addr);
   public:
      sockaddr* getAddr() override;

      const sockaddr* getAddr() const override;

      socklen_t getAddrSize() const override;

      std::ostream& insert(std::ostream& os) const override;
   private:
      sockaddr m_addr; 
};


std::ostream& operator<< (std::ostream& os,const Address& addr);
}

#endif
