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
    typedef std::weak_ptr<Socket> weak_ptr;

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
    virtual ~Socket();

    uint64_t getSendTimeout() const ;
    void setSendTimeout(const uint64_t timeout);
    uint64_t getRecvTimeout() const;
    void setRecvTimeout(const uint64_t timeout);

    template<typename T>
    bool getSockOption(int level, int optname, T&optval){
      ssize_t len = sizeof(T);
      return getSockOption(level,optname,&optval,&len);
    }
    template<typename T>
    bool setSockOption(int level, int optname, T&optval){
      ssize_t len = sizeof(T);
      return setSockOption(level,optname,&optval,len);
    }
    
    bool getSockOption(int level, int optname, void *optval, ssize_t *optlen);

    bool setSockOption(int level, int optname, const void *optval, ssize_t optlen);

    virtual Socket::ptr accept();

    virtual bool bind(const Address::ptr addr);

    virtual bool connect(const Address::ptr addr,uint64_t timeout_ms = -1);

    virtual bool reconnect(uint64_t timeout_ms = -1);

    virtual bool listen(int backlog = SOMAXCONN);

    virtual bool close();

    virtual int send(const void* buffer,size_t length,int flags = 0);

    virtual int send(const iovec* buffers, size_t length, int flags = 0);

    virtual int sendTo(const void* buffer, size_t length,const Address::ptr addrTo ,int flags = 0);

    virtual int sendTo(const iovec* buffers, size_t length,const Address::ptr addrto ,int flags = 0);

    virtual int recv(void* buffer, size_t length, int flags = 0);

    virtual int recv(iovec* buffers, size_t length, int flags = 0);

    virtual int recvFrom(void* buffer, size_t length, Address::ptr addrFrom, int flags = 0);

    virtual int recvFrom(iovec* buffer, size_t length, Address::ptr addrFrom, int flags = 0);

    Address::ptr getRemoteAddress();

    Address::ptr getLocalAddress();

    int getFamily() const { return m_family;}

    int getType() const { return m_type;}

    int getProtocol() const { return m_protocol;}  

    bool isConnected()const{return m_isConnected;}

    bool isValid() const;

    int getError();

    bool cancelRead();

    bool cancelWrite();

    bool cancelAccept();

    bool cancelAll();

    virtual std::ostream& insert(std::ostream& os) const;

    virtual std::string toString() const;
  protected:
    //将sockfd设为so_reuseaddr
    void initSock();

    void newSock();

    virtual bool init(int sock);
  private:
    int m_sockfd;

    int m_family;

    int m_type;

    int m_protocol;

    int m_isConnected;

    Address::ptr m_remoteAddress;

    Address::ptr m_localAddress;
  
};

std::ostream& operator<<(std::ostream& os , Socket::ptr sock);



}


#endif
