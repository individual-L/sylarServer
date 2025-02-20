#include"socket.hpp"
#include"fdMng.hpp"
#include"macro.hpp"
#include"hook.hpp"
#include"log.hpp"
#include"iomanager.hpp"

#include<netinet/in.h>
#include<netinet/tcp.h>

static gaiya::Logger::ptr logger = LOG_M()->getLogger("master");

namespace gaiya{


Socket::Socket(int family,int type, int protocol)
:m_sockfd(-1)
,m_family(family)
,m_type(type)
,m_protocol(protocol)
,m_isConnected(false){
}

Socket::ptr Socket::CreateTCP(Address::ptr addr){
  Socket::ptr sock(new Socket(addr->getFamily(),TCP,0));
  return sock;
}

Socket::ptr Socket::CreateUDP(Address::ptr addr){
  Socket::ptr sock(new Socket(addr->getFamily(),UDP,0));
  return sock;
}
Socket::ptr Socket::CreateUDPScoket(){
  Socket::ptr sock(new Socket(IPv4,UDP,0));
  return sock;
}
Socket::ptr Socket::CreateTCPScoket(){
  Socket::ptr sock(new Socket(IPv4,TCP,0));
  return sock;
}

Socket::ptr Socket::CreateUDPScoket6(){
  Socket::ptr sock(new Socket(IPv6,UDP,0));
  return sock;
}
Socket::ptr Socket::CreateTCPScoket6(){
  Socket::ptr sock(new Socket(IPv6,TCP,0));
  return sock;
}

Socket::ptr Socket::CreateUnixTCPScoket(){
  Socket::ptr sock(new Socket(UNIX,TCP,0));
  return sock;
}
Socket::ptr Socket::CreateUnixUDPScoket(){
  Socket::ptr sock(new Socket(UNIX,TCP,0));
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

bool Socket::getSockOption(int level, int optname, void *optval, ssize_t *optlen){
  bool res = getsockopt(m_sockfd,level,optname,optval,(socklen_t*)optlen);
  if(res){
    LOG_ERROR(logger) <<"getsockopt(" <<m_sockfd<<"," <<level << "," <<optname <<","
                      <<optval<<"," <<optlen <<") errno: " <<errno <<"("<<strerror(errno)<<")";
    return false;
  }
  return true;
}

bool Socket::setSockOption(int level, int optname, const void *optval, ssize_t optlen){
  bool res = setsockopt(m_sockfd,level,optname,optval,(socklen_t)optlen);
  if(res){
    LOG_ERROR(logger) <<"setSockOption(" <<m_sockfd<<"," <<level << "," <<optname 
                      <<","<<optval<<"," <<optlen <<") errno: " <<errno 
                      <<"("<<strerror(errno)<<")";
    GAIYA_ASSERT(false);
    return false;
  }
  return true;
}

void Socket::initSock(){
  int val = 1;
  // setSockOption(SOL_SOCKET,SO_REUSEADDR,val);
  
  if(m_type == SOCK_STREAM){
    setSockOption(IPPROTO_TCP,TCP_NODELAY,val);
  }

}

void Socket::newSock(){
  m_sockfd = ::socket(m_family,m_type,m_protocol);
  if(GAIYA_LIKELY(m_sockfd != -1)){
    initSock();
  }else{
    LOG_ERROR(logger) << "socket(" << m_family
        << ", " << m_type << ", " << m_protocol << ") errno = "
        << errno << ", strerr = " << strerror(errno);
  }
}

bool Socket::init(int sock){
  gaiya::FdCtx::ptr ctx = gaiya::fdMng::getInstance()->get(sock);
  if(ctx && ctx->isSocket() && !ctx->isClose()){
    m_sockfd = sock;
    m_isConnected = true;
    initSock();
    getRemoteAddress();
    getLocalAddress();
    return true;
  }
  return false;
}
Address::ptr Socket::getRemoteAddress(){
  if(m_remoteAddress){
    return m_remoteAddress;
  }
  gaiya::Address::ptr addr;
  switch(m_family){
    case IPv4:
      addr = gaiya::Address::ptr(new gaiya::IPv4Address());
      break;
    case IPv6:
      addr = gaiya::Address::ptr(new gaiya::IPv6Address());
      break;
    case UNIX:
      addr = gaiya::Address::ptr(new gaiya::UnixAddress());
      break;
    default:
      addr = gaiya::Address::ptr(new gaiya::UknownAddress(m_family));
      break;     
  }
  socklen_t len = addr->getAddrSize();
  int res = getpeername(m_sockfd,addr->getAddr(),&len);
  if(res){
    LOG_ERROR(logger) <<"getpeername() errno: " <<strerror(errno);
    return Address::ptr(new gaiya::UknownAddress(m_family));
  }

  if(m_family == UNIX){
    gaiya::UnixAddress::ptr un = std::dynamic_pointer_cast<UnixAddress>(addr);
    un->setAddrLen(len);
  }
  m_remoteAddress = addr;
  return addr;
}

Address::ptr Socket::getLocalAddress(){
  if(m_localAddress){
    return m_localAddress;
  }
  gaiya::Address::ptr addr;
  switch(m_family){
    case IPv4:
      addr = gaiya::Address::ptr(new gaiya::IPv4Address());
      break;
    case IPv6:
      addr = gaiya::Address::ptr(new gaiya::IPv6Address());
      break;
    case UNIX:
      addr = gaiya::Address::ptr(new gaiya::UnixAddress());
      break;
    default:
      addr = gaiya::Address::ptr(new gaiya::UknownAddress(m_family));
      break;     
  }
  socklen_t len = addr->getAddrSize();
  int res = getsockname(m_sockfd,addr->getAddr(),&len);
  if(res){
    LOG_ERROR(logger) <<"getpeername() errno: " <<strerror(errno);
    return Address::ptr(new gaiya::UknownAddress(m_family));
  }

  if(m_family == UNIX){
    gaiya::UnixAddress::ptr un = std::dynamic_pointer_cast<UnixAddress>(addr);
    un->setAddrLen(len);
  }
  m_localAddress = addr;
  return addr;
}

Socket::~Socket()
{
  close();
}

Socket::ptr Socket::accept(){
  Socket::ptr sock(new Socket(m_family, m_type, m_protocol));
  int newsock = ::accept(m_sockfd, nullptr, nullptr);
  // LOG_INFO(logger)
  if(newsock == -1) {
      LOG_ERROR(logger) << "accept(" << m_sockfd << ") errno="
          << errno << " errstr=" << strerror(errno);
      return nullptr;
  }
  LOG_INFO(logger) <<"accept client fd: "<<newsock;
  if(sock->init(newsock)) {
      return sock;
  }
  return nullptr;
}

bool Socket::bind(const Address::ptr addr){
    if(!isValid()) {
        newSock();
        if(GAIYA_UNLIKELY(!isValid())) {
            return false;
        }
    }

    if(GAIYA_UNLIKELY(addr->getFamily() != m_family)) {
        LOG_ERROR(logger) << "bind sock.family("
            << m_family << ") addr.family(" << addr->getFamily()
            << ") not equal, addr=" << addr->toString();
        return false;
    }

    //addr为unix套接字
    UnixAddress::ptr un = std::dynamic_pointer_cast<UnixAddress>(addr);
    if(un){
      LOG_INFO(logger)<<"unixSocket created";
      Socket::ptr sock = CreateUnixTCPScoket();
      if(sock->connect(un)){
        return false;
      }else{
        //删除已存在的同名文件
        gaiya::Unlink(un->getPath());
      }
    }

    if(::bind(m_sockfd, addr->getAddr(), addr->getAddrSize())) {
        LOG_ERROR(logger) << "bind error errrno=" << errno
            << " errstr=" << strerror(errno);
        return false;
    }
    getLocalAddress();
    return true;   
}

bool Socket::connect(const Address::ptr addr,uint64_t timeout_ms ){
  if(!isValid()){
    newSock();
    if(!GAIYA_LIKELY(isValid())) {
        return false;
    }
  }
  if(GAIYA_UNLIKELY(addr->getFamily() != m_family)){
    LOG_ERROR(logger) << "connect sock.family("
        << m_family << ") addr.family(" << addr->getFamily()
        << ") not equal, addr=" << addr->toString();
    return false;
  }

  if(timeout_ms == (uint64_t)-1){
    if(::connect(m_sockfd,addr->getAddr(),addr->getAddrSize())){
      LOG_ERROR(logger) << "sock=" << m_sockfd << " connect(" << addr->toString()
          << ") error errno=" << errno << " errstr=" << strerror(errno);
      close();
      return false;
    }
  }else{
    if(::connect_with_timeout(m_sockfd,addr->getAddr(),addr->getAddrSize(),timeout_ms)){
      LOG_ERROR(logger) << "sock=" << m_sockfd << " connect(" << addr->toString()
          << ") error errno=" << errno << " errstr=" << strerror(errno);
      close();
      return false;
    }
  }
  m_isConnected = true;
  getLocalAddress();
  getRemoteAddress();
  return true;
}

bool Socket::reconnect(uint64_t timeout_ms){
  if(!m_remoteAddress){
    LOG_ERROR(logger) << "reconnect m_remoteAddress is null";
    return false;    
  }
  m_localAddress.reset();
  return connect(m_remoteAddress,timeout_ms);
}

bool Socket::listen(int backlog){
  if(!isValid()){
    LOG_ERROR(logger) << "listen error sock=-1";
    return false; 
  }
  if(::listen(m_sockfd,backlog)){
    LOG_ERROR(logger) << "listen error errno=" << errno
        << " errstr=" << strerror(errno);
    return false;   
  }
  return true;
}

int Socket::send(const void* buffer,size_t length,int flags){
  if(isConnected()){
    return ::send(m_sockfd,buffer,length,flags);
  }
  return -1;
}

int Socket::send(const iovec* buffers, size_t length, int flags){
  if(isConnected()){
    struct msghdr msg;
    memset(&msg,0,sizeof(msg));
    msg.msg_iov = (iovec*)buffers;
    msg.msg_iovlen = length;
    return ::sendmsg(m_sockfd,&msg,flags);
  }
  return -1;
}

int Socket::sendTo(const void* buffer, size_t length,const Address::ptr addrTo ,int flags){
  if(isConnected()){
    return ::sendto(m_sockfd,buffer,length,flags,addrTo->getAddr(),addrTo->getAddrSize());
  }
  return -1;
}

int Socket::sendTo(const iovec* buffers, size_t length,const Address::ptr addrto ,int flags){
  if(isConnected()){
    struct msghdr msg;
    memset(&msg,0,sizeof(msg));
    msg.msg_iov = (iovec*)buffers;
    msg.msg_iovlen = length;
    msg.msg_name = addrto->getAddr();
    msg.msg_namelen = addrto->getAddrSize();
    return ::sendmsg(m_sockfd,&msg,flags);
  }
  return -1;
}

int Socket::recv(void* buffer, size_t length, int flags){
  if(isConnected()) {
      return ::recv(m_sockfd, buffer, length, flags);
  }
  return -1;
}

int Socket::recv(iovec* buffers, size_t length, int flags){
  if(isConnected()) {
    struct msghdr msg;
    memset(&msg,0,sizeof(msg));
    msg.msg_iov = (iovec*)buffers;
    msg.msg_iovlen = length;
    return ::recvmsg(m_sockfd,&msg,flags);
  }
  return -1;
}

int Socket::recvFrom(void* buffer, size_t length, Address::ptr addrFrom, int flags){
  if(isConnected()) {
    socklen_t len = addrFrom->getAddrSize();
    return ::recvfrom(m_sockfd,buffer,length,flags,addrFrom->getAddr(),&len);
  }
  return -1;
}

int Socket::recvFrom(iovec* buffer, size_t length, Address::ptr addrFrom, int flags){
  if(isConnected()) {
    struct msghdr msg;
    memset(&msg,0,sizeof(msg));
    msg.msg_iov = (iovec*)buffer;
    msg.msg_iovlen = length;
    msg.msg_name = addrFrom->getAddr();
    msg.msg_namelen = addrFrom->getAddrSize();
    return ::recvmsg(m_sockfd,&msg,flags);
  }
  return -1;
}


bool Socket::isValid() const {
  return m_sockfd != -1;
}


bool Socket::close(){
  if(!isConnected() && m_sockfd == -1){
    return false;
  }
  if(m_sockfd != -1){
    // LOG_INFO(logger)<<"fd: "<<m_sockfd<<" closed";
    ::close(m_sockfd);
    m_sockfd = -1;
  }
  return true;
}


int Socket::getError(){
  int error = 0;
  if(!getSockOption(SOL_SOCKET,SO_ERROR,error)){
    error = errno;
  }
  return error;
}

bool Socket::cancelRead(){
  return gaiya::IOmanager::GetThis()->cancelEvent(m_sockfd,IOmanager::READ);
}

bool Socket::cancelWrite(){
  return gaiya::IOmanager::GetThis()->cancelEvent(m_sockfd,IOmanager::WRITE);

}

bool Socket::cancelAccept(){
  return gaiya::IOmanager::GetThis()->cancelEvent(m_sockfd,IOmanager::READ);
}

bool Socket::cancelAll(){
  return gaiya::IOmanager::GetThis()->cancelAll(m_sockfd);
}

std::ostream& Socket::insert(std::ostream& os) const{
  os <<std::endl 
      << "[Socket sock: " << m_sockfd
      << " is_connected: " << m_isConnected
      << " family: " << m_family
      << " type: " << m_type
      << " protocol: " << m_protocol;
    if(m_localAddress) {
        os << " local_address: " << m_localAddress->toString();
    }
    if(m_remoteAddress) {
        os << " remote_address: " << m_remoteAddress->toString();
    }
    os << "]";
    return os;
}

std::string Socket::toString() const{
  std::stringstream ss;
  insert(ss);
  return ss.str();
}

std::ostream& operator<<(std::ostream& os , Socket::ptr sock){
  return sock->insert(os);
}

}
