#include"fdMng.hpp"
#include <sys/stat.h>
#include<sys/socket.h>
#include<fcntl.h>
#include"macro.hpp"
#include"log.hpp"

static gaiya::Logger::ptr logger = LOG_GET_LOGGER("master");

namespace gaiya{

FdCtx::FdCtx(int fd):
m_fd(fd)
,m_isInit(false)
,m_isSocket(false)
,m_isClosed(false)
{
  init();
}

bool FdCtx::init(){
  m_writeTimeOut = -1;
  m_readTimeOut = -1;
  if(m_isInit){
    return false;
  }
  struct stat fdStat;
  //检测此fd是否为sockfd
  if(fstat(m_fd,&fdStat) == -1){
    m_isSocket = false;
    m_isInit = false;
  }else{
    m_isInit = true;
    m_isSocket = S_ISSOCK(fdStat.st_mode);
  }
  //设置为非阻塞
  if(m_isSocket){
    int flag = fcntl(m_fd,F_GETFL,0);
    if(!(flag & O_NONBLOCK)){
      flag |= O_NONBLOCK;
      int res = fcntl(m_fd,F_SETFL,flag);
      if(res){
        LOG_ERROR(logger) <<"FdCtx::init()::fcntl res = " <<res;
        GAIYA_ASSERT(false);
      }
      m_sysNonBlock = true;
    }else{
      m_sysNonBlock = false;
    }
  }

  m_userNonBlock = false;
  m_isClosed = false;
  return m_isInit;
}
void FdCtx::setTimeout(int type, uint64_t v){
  switch (type)
  {
  case 0x1:
    m_readTimeOut = v;
    break;
  case 0x04:
    m_writeTimeOut = v;
  }
}

uint64_t FdCtx::getTimeout(int type){
  switch (type)
  {
    //gaiya::IOmanager::READ
  case SO_RCVTIMEO:
    return m_readTimeOut;
    //gaiya::IOmanager::WRITE
  case SO_SNDTIMEO:
    return m_writeTimeOut;
  }
  return -1;
}

FdMng::FdMng(){
  m_fds.resize(64);
}

FdCtx::ptr FdMng::get(int fd, bool auto_create){
  if(fd == -1){
    return nullptr;
  }
  mutexType::ReadLock lock(m_mutex);
  if(m_fds.size() <= (size_t)fd){
    if(auto_create == false){
      return nullptr;
    }
  }else{
    if(m_fds[fd] || !auto_create){
      return m_fds[fd];
    }
  }
  lock.unlock();

  FdCtx::ptr fd_ptr(new FdCtx(fd));
  mutexType::WriteLock lock2(m_mutex);
  if(m_fds.size() <= (size_t)fd){
    m_fds.resize(fd * 1.5);
  }
  m_fds[fd] = fd_ptr;
  return fd_ptr;
}

void FdMng::del(int fd){
  mutexType::WriteLock lock(m_mutex);
  if(m_fds.size() <= (size_t)fd){
    return;
  }
  m_fds[fd].reset();
}


}
