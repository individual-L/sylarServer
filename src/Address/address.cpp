#include"address.hpp"
#include"log.hpp"
#include <sys/types.h>
#include"util.hpp"
#include"byteSwapGaiya.hpp"
#include <netdb.h>
#include <ifaddrs.h>
#include<utility>



static gaiya::Logger::ptr logger = LOG_M()->getLogger("master");
namespace gaiya{

template<typename T>
//计算val的字节序列中置为1的bit个数
static uint32_t CountBits(T val){
  uint32_t res = 0;
  for(;val;++res){
    val &= val - 1;
  }
  return res;
}

template<typename T>
//count: 子网掩码的占的bit数
static T CreateMask_r(uint32_t count){
  return (1 << (sizeof(T) * 8 - count)) - 1;
}

Address::ptr Address::Create(const sockaddr* sockaddr,const socklen_t sockLen){
  if(sockaddr == nullptr){
    return nullptr;
  }
  Address::ptr addr = nullptr;
  switch (sockaddr->sa_family)
  {
  case AF_INET:
    addr.reset(new IPv4Address(*(const sockaddr_in *)sockaddr));
    break;
  case AF_INET6:
    addr.reset(new IPv6Address(*(const sockaddr_in6 *)sockaddr));
    break;
  default:
    addr.reset(new UknownAddress(*sockaddr));
    break;
  }
  return addr;
}

bool Address::operator==(const Address& addr) const {
  return getAddrSize() == addr.getAddrSize() &&
          memcmp(getAddr(),addr.getAddr(),getAddrSize());
}

bool Address::operator<(const Address& addr) const {
  size_t len = getAddrSize() < addr.getAddrSize() ? getAddrSize() : addr.getAddrSize();
  return memcmp(getAddr(),addr.getAddr(),len);
}

bool Address::operator!=(const Address& addr) const {
  return !(addr == *this);
}

//返回host的所有符合条件的Address
bool Address::Lookup(std::vector<Address::ptr>& addresses, const std::string& host,int family = AF_INET,int type = 0,int protocol = 0){
  struct addrinfo hints , *res , *next;

  hints.ai_family = family;
  hints.ai_protocol = protocol;
  hints.ai_socktype = type;
  hints.ai_addr = nullptr;
  hints.ai_addrlen = 0;
  hints.ai_canonname = NULL;
  hints.ai_flags = 0;
  hints.ai_next = nullptr;

  std::string node = nullptr;
  const char* port = 0;

  //ipv6格式: [ip]:port
  if(!host.empty() && host[0] == '['){
    const char * p = (const char*)memchr(host.c_str() + 1,']',sizeof(host) - 1);

    if(p){
      if(*(p + 1) == ':'){
        port = p + 2;
      }
    }
    node = host.substr(1,p - host.c_str() - 1);
  }
  
  //ipv6格式: ip:port
  if(!host.empty()){
    port = (const char*)memchr(host.c_str(),':',sizeof(host));
    if(port){
      node = host.substr(0,port - host.c_str());
      ++port;
    }
  }

  if(host.empty()){
    node = host;
  }
  int error = getaddrinfo(node.c_str(),port,&hints,&res);

  if(error){
    LOG_ERROR(logger) <<"Address::Lookup(" <<host <<", "<<family <<", " <<type <<", "
                      <<protocol<<") error = " <<error <<"(" <<strerror(error) <<")";
    return false;
  }

  next = res;
  while(next){
    addresses.push_back(Create(next->ai_addr,next->ai_addrlen));
    next = next->ai_next;
  }
  freeaddrinfo(res);
  return !addresses.empty();
}

//返回host的任意符合条件的Address
Address::ptr Address::LookupAny(const std::string& hostName,int family = AF_INET,int type = 0,int protocol = 0){
  std::vector<Address::ptr> addresses;
  if(Lookup(addresses,hostName,family,type,protocol)){
    return addresses[0];
  }
  return nullptr;
}

//返回host的任意符合条件的Address
IPAddress::ptr Address::LookupAnyIPAddress(const std::string& hostName,int family = AF_INET,int type = 0,int protocol = 0){
  std::vector<Address::ptr> addresses;
  IPAddress::ptr res = nullptr;
  if(Lookup(addresses,hostName,family,type,protocol)){
    for(auto& it : addresses){
      res = std::dynamic_pointer_cast<IPAddress>(it);
      if(res){
        return res;
      }
    }
  }
  return nullptr;
}

//
bool Address::GetInterfaceAddress(std::multimap<std::string,std::pair<Address::ptr,uint32_t>>& results,int family = AF_INET){

  struct ifaddrs *res, *next;
  int error = getifaddrs(&res);
  if(error){
    LOG_ERROR(logger) <<"Address::GetInterfaceAddress(" <<family<<") error = " <<error <<"("<<strerror(error)<<")";
    return false;
  }

  try{
    for(next = res;next;next = next = next->ifa_next){
      if(family != AF_UNSPEC && family != next->ifa_addr->sa_family){
        continue;
      }
      Address::ptr addr;
      uint32_t prefix_len = 0;

      switch (next->ifa_addr->sa_family)
      {
      case AF_INET:
      {
        addr = Create(next->ifa_addr,sizeof(next->ifa_addr));
        uint32_t mask = ((sockaddr_in*)next->ifa_netmask)->sin_addr.s_addr;
        prefix_len = CountBits(mask);
        break;
      }
      case AF_INET6:
      {
        addr = Create(next->ifa_addr,sizeof(next->ifa_addr));
        uint8_t *mask = ((sockaddr_in6*)next->ifa_netmask)->sin6_addr.s6_addr;
        for(int i = 0;i < 16;++i){
          prefix_len += CountBits(mask[i]);
        }
        break;
      }
      default:
        break;
      }
      if(addr){
        results.insert(std::make_pair<std::string,std::pair<Address::ptr,uint32_t>>(next->ifa_name,std::make_pair(addr,prefix_len)));
      }

    }
  }catch(...){
    LOG_ERROR(logger) <<"Address::GetInterfaceAddress";
    freeifaddrs(res);
    return false;
  }
  freeifaddrs(res);
  return false; 
}

bool Address::GetInterfaceAddress(std::vector<std::pair<Address::ptr,uint32_t>>& res,const std::string& ifname,int family = AF_INET){

  if (ifname.empty() || ifname == "*") {
      //	创建监听任意IP地址的连接请求的ipv4
      if (family == AF_INET || family == AF_UNSPEC) {
          res.push_back(std::make_pair(Address::ptr(new gaiya::IPv4Address()), 0u));
      }
      //	创建监听任意IP地址的连接请求的ipv6
      if (family == AF_INET6 || family == AF_UNSPEC) {
          res.push_back(std::make_pair(Address::ptr(new gaiya::IPv6Address()), 0u));
      }
      return true;
  }
  std::multimap<std::string,std::pair<Address::ptr,uint32_t>> results;
  if(!GetInterfaceAddress(results,family)){
    return false;
  }
  auto it = results.equal_range(ifname);
  for(;it.first != it.second;++(it.first)){
    res.push_back(std::make_pair(it.first->second.first,it.first->second.second));
  }
  return true;
}


int Address::getFamily() const {
  return getAddr()->sa_family;
}

std::string Address::toString() const {
  std::stringstream ss;
  insert(ss);
  return ss.str();
}

IPv4Address::IPv4Address(const sockaddr_in & addr)
:m_addr(addr){

}

IPv4Address::IPv4Address(uint32_t addr,uint16_t port){
  memset(&m_addr,0,sizeof(m_addr));
  m_addr.sin_family= AF_INET;
  m_addr.sin_port = gaiya::byteSwapHN(port);
  m_addr.sin_addr.s_addr = gaiya::byteSwapHN(addr);
}

const sockaddr* IPv4Address::getAddr() const{
  return (sockaddr*) &m_addr;
}

const socklen_t IPv4Address::getAddrSize() const {
  return sizeof(m_addr);
}

std::ostream& IPv4Address::insert(std::ostream& os) const {
  uint32_t addr = byteSwapHN(m_addr.sin_addr.s_addr);
  os << ((addr >> 24) & 0xff) << "."
    << ((addr << 16) & 0xff) << "."
    << ((addr >> 8) & 0xff) << "."
    << (addr & 0xff);
  os << ":" <<byteSwapHN(m_addr.sin_port);
  return os;
}

uint16_t IPv4Address::getPort() const {
  return gaiya::byteSwapHN(m_addr.sin_port);
}

void IPv4Address::setPort(uint16_t port) {
  m_addr.sin_port = gaiya::byteSwapHN(port);
}


IPv6Address::IPv6Address(){
  memset(&m_addr,0,sizeof(m_addr));
  m_addr.sin6_family = AF_INET6;
}

IPv6Address::IPv6Address(const sockaddr_in6& addr)
:m_addr(addr){

}

IPv6Address::IPv6Address(const uint8_t addr[16],uint16_t port){
  memset(&m_addr,0,sizeof(m_addr));
  m_addr.sin6_family = AF_INET6;
  m_addr.sin6_port = gaiya::byteSwapHN(port);
  memcpy(&m_addr.sin6_addr.s6_addr,addr,16);
}

const sockaddr* IPv6Address::getAddr() const{
  return (sockaddr*) &m_addr;
}

const socklen_t IPv6Address::getAddrSize() const {
  return sizeof(m_addr);
}

uint16_t IPv6Address::getPort() const {
  return gaiya::byteSwapHN(m_addr.sin6_port);
}

void IPv6Address::setPort(uint16_t port) {
  m_addr.sin6_port = gaiya::byteSwapHN(port);
}

std::ostream& IPv6Address::insert(std::ostream& os) const {
  os <<"[";
  uint16_t * addr = (uint16_t*) m_addr.sin6_addr.s6_addr;
  bool usedZero = false;
  for(size_t i = 0;i < 8;++i){
    if(addr[i] == 0 && usedZero){
      continue;
    }
    if(addr[i] == 0){
      os << 0 <<":";
      usedZero = true;
      continue;
    }
    os <<std::hex << (int)gaiya::byteSwapHN(addr[i]) << std::dec;
    if(i < 7){
      os <<":";
    }
  }
  os <<"]:" <<gaiya::byteSwapHN(m_addr.sin6_port);
  return os;
}
}
