#ifndef __SOCKET_H__
#define __SOCKET_H__

#include<sys/socket.h>
#include<memory>

#include"address.hpp"
#include"Noncopyable.hpp"

namespace gaiya{

class Socket :public std::enable_shared_from_this<Socket>, Noncopyable
{
  public:
    typedef std::shared_ptr<Socket> ptr;
    typedef std::weak_ptr<Socket> ptr;

    enum FamilyType{
      IPv4 = AF_INET,
      IPv6 = AF_INET6,
      UNIX = AF_UNIX
    };
    enum SockType{
      TCP = SOCK_STREAM,
      UDP = SOCK_DGRAM
    };

  public:
    static Socket::ptr CreateTCP(Address::ptr addr);

    static Socket::ptr CreateUDP(Address::ptr addr);
    static Socket::ptr CreateUDPScoket();
    static Socket::ptr CreateTCPScoket();

    static Socket::ptr CreateUDPScoket6();
    static Socket::ptr CreateTCPScoket6();

    static Socket::ptr CreateUnixTCPScoket();
    static Socket::ptr CreateUnixUDPScoket();

  
  public:
    Socket(/* args */){}
    Socket(int family,int type, int protocol);
    ~Socket();

    uint64_t getSendTimeout() const ;
    void setSendTimeout(const uint64_t timeout);
    uint64_t getRecvTimeout() const;
    void setRecvTimeout(const uint64_t timeout);
  private:
    bool close();
  private:
    int m_sockfd;

    int m_family;

    int m_type;

    int m_protocol;

    int isConnected;

    Address::ptr m_remoteAddress;

    Address::ptr m_localAddress;
  
};





}


#endif
