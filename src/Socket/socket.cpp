#include"socket.hpp"
#include"fdMng.hpp"
#include<netinet/in.h>
namespace gaiya{


Socket::Socket(int family,int type, int protocol)
:m_sockfd(-1)
,m_family(family)
,m_type(type)
,m_protocol(protocol)
,isConnected(false){
}

Socket::ptr Socket::CreateTCP(Address::ptr addr){
  Socket::ptr sock(new Socket(addr->getFamily(),TCP,IPPROTO_TCP));
  return sock;
}

Socket::ptr Socket::CreateUDP(Address::ptr addr){
  Socket::ptr sock(new Socket(addr->getFamily(),UDP,IPPROTO_UDP));
  return sock;
}
Socket::ptr Socket::CreateUDPScoket(){
  Socket::ptr sock(new Socket(IPv4,UDP,IPPROTO_UDP));
  return sock;
}
Socket::ptr Socket::CreateTCPScoket(){
  Socket::ptr sock(new Socket(IPv4,TCP,IPPROTO_TCP));
  return sock;
}

Socket::ptr Socket::CreateUDPScoket6(){
  Socket::ptr sock(new Socket(IPv6,UDP,IPPROTO_UDP));
  return sock;
}
Socket::ptr Socket::CreateTCPScoket6(){
  Socket::ptr sock(new Socket(IPv6,TCP,IPPROTO_TCP));
  return sock;
}

Socket::ptr Socket::CreateUnixTCPScoket(){
  Socket::ptr sock(new Socket(UNIX,TCP,IPPROTO_TCP));
  return sock;
}
Socket::ptr Socket::CreateUnixUDPScoket(){
  Socket::ptr sock(new Socket(UNIX,TCP,IPPROTO_TCP));
  return sock;
}


uint64_t Socket::getSendTimeout() const {
  gaiya::FdCtx::ptr fd =  gaiya::fdMng::getInstance()->get(m_sockfd);
  if(fd){
    return fd->getTimeout(SO_SNDTIMEO);
  }
  return -1;
}
void Socket::setSendTimeout(const uint64_t timeout){
  gaiya::FdCtx::ptr fd =  gaiya::fdMng::getInstance()->get(m_sockfd);
  if(fd){
    fd->setTimeout(SO_SNDTIMEO,timeout);
  }
}
uint64_t Socket::getRecvTimeout() const {
  gaiya::FdCtx::ptr fd =  gaiya::fdMng::getInstance()->get(m_sockfd);
  if(fd){
    return fd->getTimeout(SO_RCVTIMEO);
  }
  return -1;
}
void Socket::setRecvTimeout(const uint64_t timeout){
  gaiya::FdCtx::ptr fd =  gaiya::fdMng::getInstance()->get(m_sockfd);
  if(fd){
    fd->setTimeout(SO_RCVTIMEO,timeout);
  }
}

Socket::~Socket()
{
  close();
}

bool Socket::close(){
  if(!isConnected && m_sockfd != -1){
    return false;
  }
  ::close(m_sockfd);
  return true;
}

}
