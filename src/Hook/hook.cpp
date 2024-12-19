#include"hook.hpp"
#include"coroutine.hpp"
#include"iomanager.hpp"
#include"schedule.hpp"
#include"fdMng.hpp"

#include <dlfcn.h>
#include<fcntl.h>
#include<functional>
#include <sys/uio.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

static gaiya::Logger::ptr logger = LOG_GET_LOGGER("master");



namespace gaiya{

static uint64_t s_timeout = -1;

static gaiya::ConfigVar<uint64_t>::ptr s_timeout_ms = gaiya::Config::lookup("tcp.connect.timeout_ms","tcp.connect.timeout_ms",5000ul);

static thread_local bool t_isHook = false;

bool isHook(){
  return t_isHook;
}

void setThreadHook(bool isHook){
  t_isHook = isHook;
}

#define HOOK_INIT(XX) \
  XX(sleep) \
  XX(usleep) \
  XX(nanosleep) \
  XX(socket) \
  XX(connect) \
  XX(accept) \
  XX(read) \
  XX(readv) \
  XX(recv) \
  XX(recvfrom) \
  XX(recvmsg) \
  XX(write) \
  XX(writev) \
  XX(send) \
  XX(sendto) \
  XX(sendmsg) \
  XX(close) \
  XX(fcntl) \
  XX(ioctl) \
  XX(getsockopt) \
  XX(setsockopt)


void hook_init(){
  static bool isInit = false;

  s_timeout = s_timeout_ms->getValue();

  s_timeout_ms->addCallBackFunc([](const int& oldData,const int& newData){
    LOG_INFO(logger) <<" timeout_ms changed to newData: "<< newData <<" from oldData: "<<oldData;
    s_timeout = newData;
  });
  if(isInit){
    return;
  }
#define XX(name) \
  name ## _f = (name ## _fun)dlsym(RTLD_NEXT, #name);
  HOOK_INIT(XX)
#undef XX


}

struct HookIniter{
  HookIniter(){
    hook_init();
  }
};

static HookIniter hookiniter;

}

struct timerCond{
  int cancelled = 0;
};

template<typename OriginFunc,typename... Args>
static ssize_t do_io(int fd,OriginFunc fun,const char * name_f,uint32_t event,int so_timeOut,Args&&... args){
  GAIYA_ASSERT2((!!fun),"fun not exist");

  if(!gaiya::isHook()){
    return fun(fd,std::forward<Args>(args)...);
  }
  gaiya::FdCtx::ptr fdctx = gaiya::fdMng::getInstance()->get(fd);

  if(!fdctx){
    return fun(fd,std::forward<Args>(args)...);
  }

  if(fdctx->isClose()) {
      errno = EBADF;
      return -1;
  }

  //不是socket或者非阻塞立即返回
  if(!fdctx->isSocket() || fdctx->getUserNonBlock()){
    return fun(fd,std::forward<Args>(args)...);
  }

  ssize_t n = 0;

  uint64_t timeOut = fdctx->getTimeout(so_timeOut);
  gaiya::Timer::ptr timer;
  std::shared_ptr<timerCond> tcond(new timerCond());
  std::weak_ptr<timerCond> wcond(tcond);
  gaiya::IOmanager* iom = gaiya::IOmanager::GetThis();

retry:
  n = fun(fd,std::forward<Args>(args)...);

  while(n == -1 && errno ==  EINTR){
    n = fun(fd,std::forward<Args>(args)...);
  }
  if(n == -1 && errno == EAGAIN){

    //是否设置超时时间
    if(timeOut != (uint64_t)-1){
      timer = iom->addConditionTimer(timeOut,[fd,wcond,iom](){
      auto it = wcond.lock();
      if(!it || it->cancelled){
        return;
      }
      it->cancelled = ETIMEDOUT;
      iom->cancelEvent(fd,gaiya::IOmanager::READ);
      },wcond,false);
    }
    //添加监听事件，回调函数为此协程
    bool ok = iom->addEvent(fd,gaiya::IOmanager::READ);

    if(GAIYA_LIKELY(ok)){
      gaiya::Coroutine::YieldToHold();
      if(timer){
        timer->cancelTimer();
      }
      if(tcond->cancelled){
        errno = ETIMEDOUT;
        return -1;
      }
      LOG_INFO(logger) <<"读事件触发";
    }else{
      LOG_ERROR(logger) <<"name_f::iom->addEvent(" <<fd <<",gaiya::IOmanager::READ) error";
      if(timer){
        timer->cancelTimer();
      }
      return -1;
    }
    goto retry;
  }
  return n;
}



extern "C"{

#define XX(name) name ## _fun name ## _f = nullptr;
  HOOK_INIT(XX)
#undef XX

unsigned int sleep(unsigned int seconds){
  if(!gaiya::isHook()){
    return sleep_f(seconds);
  }
  gaiya::Coroutine::ptr coro = gaiya::Coroutine::GetCurCoro();
  gaiya::IOmanager* iom = gaiya::IOmanager::GetThis();
  iom->addTimer(seconds * 1000
  ,std::bind((void(gaiya::Scheduler::*)(gaiya::Coroutine::ptr,int thread))
            &gaiya::IOmanager::schedule
  ,iom,coro, -1));
  gaiya::Coroutine::YieldToHold();
  return 0;
}

int usleep(useconds_t useconds){
  if(!gaiya::isHook()){
    return usleep_f(useconds);
  }
  gaiya::Coroutine::ptr coro = gaiya::Coroutine::GetCurCoro();
  gaiya::IOmanager* iom = gaiya::IOmanager::GetThis();
  iom->addTimer(useconds / 1000
  ,std::bind((void(gaiya::Scheduler::*)(gaiya::Coroutine::ptr&,int thread))
            &gaiya::IOmanager::schedule
  ,iom, coro, -1));
  gaiya::Coroutine::YieldToHold();
  return 0;
}

int nanosleep(const struct timespec *duration,struct timespec *rem){
  if(!gaiya::isHook()){
  return nanosleep_f(duration,rem);
  }
  gaiya::Coroutine::ptr coro = gaiya::Coroutine::GetCurCoro();
  gaiya::IOmanager* iom = gaiya::IOmanager::GetThis();
  iom->addTimer(duration->tv_sec * 1000 + duration->tv_nsec / 1000 /1000
  ,std::bind((void(gaiya::Scheduler::*)(gaiya::Coroutine::ptr&,int thread))
            &gaiya::IOmanager::schedule
  ,iom, coro, -1));
  gaiya::Coroutine::YieldToHold();
  return 0;
}

ssize_t read(int fd, void* buf, size_t count){

  return do_io(fd,read_f,"read",gaiya::IOmanager::READ,SO_RCVTIMEO,buf,count);
}

ssize_t recv(int sockfd, void* buf, size_t len,int flags){
  return do_io(sockfd,recv_f,"recv",gaiya::IOmanager::READ,SO_RCVTIMEO,buf,len,flags);
}

ssize_t recvfrom(int sockfd, void* buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen){
  return do_io(sockfd,recvfrom_f,"recvfrom",gaiya::IOmanager::READ,SO_RCVTIMEO
              ,buf,len,flags,src_addr,addrlen);
}

ssize_t recvmsg(int sockfd, struct msghdr *msg, int flags){
  return do_io(sockfd,recvmsg_f,"recvmsg",gaiya::IOmanager::READ,SO_RCVTIMEO,msg,flags);
}

ssize_t readv(int fd, const struct iovec *iov, int iovcnt){
  return do_io(fd,readv_f,"readv",gaiya::IOmanager::READ,SO_RCVTIMEO,iov,iovcnt);
}

ssize_t write(int fd, const void *buf, size_t count) {
    return do_io(fd, write_f, "write", gaiya::IOmanager::WRITE, SO_SNDTIMEO, buf, count);
}

ssize_t writev(int fd, const struct iovec *iov, int iovcnt) {
    return do_io(fd, writev_f, "writev", gaiya::IOmanager::WRITE, SO_SNDTIMEO, iov, iovcnt);
}

ssize_t send(int s, const void *msg, size_t len, int flags) {
    return do_io(s, send_f, "send", gaiya::IOmanager::WRITE, SO_SNDTIMEO, msg, len, flags);
}

ssize_t sendto(int s, const void *msg, size_t len, int flags, const struct sockaddr *to, socklen_t tolen) {
    return do_io(s, sendto_f, "sendto", gaiya::IOmanager::WRITE, SO_SNDTIMEO, msg, len, flags, to, tolen);
}

ssize_t sendmsg(int s, const struct msghdr *msg, int flags) {
    return do_io(s, sendmsg_f, "sendmsg", gaiya::IOmanager::WRITE,SO_SNDTIMEO, msg, flags);
}

int connect_with_timeout(int fd, const struct sockaddr* addr, socklen_t addrlen, uint64_t timeout_ms){
  if(!gaiya::isHook()){
    return connect_f(fd,addr,addrlen);
  }
  gaiya::FdCtx::ptr fdCtx = gaiya::fdMng::getInstance()->get(fd);

  if(!fdCtx || fdCtx->isClose() || !fdCtx->isSocket()){
    return connect_f(fd,addr,addrlen);
  }

  if(fdCtx->getUserNonBlock()){
    return connect_f(fd,addr,addrlen);
  }

  //由于默认设置成非阻塞，故会立即返回，可能会报
  int res = connect_f(fd,addr,addrlen);
  if(res == 0){
    return 0;
  }else if(errno != EINPROGRESS){
    return res;
  }else if(errno == EINPROGRESS){
    LOG_INFO(logger) <<"正在连接中";
  }

  gaiya::IOmanager* iom = gaiya::IOmanager::GetThis();


  std::shared_ptr<timerCond> tcond(new timerCond());
  gaiya::Timer::ptr timer;
  std::weak_ptr<timerCond> wcond(tcond);

  if(timeout_ms != (uint64_t)-1){
    iom->addConditionTimer(timeout_ms,[wcond,iom,fd](){
      auto it = wcond.lock();
      if(!it || it->cancelled){
        return;
      }
      it->cancelled = ETIMEDOUT;
      LOG_INFO(logger) << "超时取消并触发事件函数";
      iom->cancelEvent(fd,gaiya::IOmanager::WRITE);
    },wcond);
  }

  bool rt = iom->addEvent(fd,gaiya::IOmanager::WRITE);
  if(rt){
    LOG_INFO(logger) <<"添加write事件成功";
    gaiya::Coroutine::YieldToHold();
    if(timer){
      timer->cancelTimer();
    }
    LOG_INFO(logger) <<"重新回到connect";
    //判断是否是超时触发
    if(tcond->cancelled){
      LOG_INFO(logger) <<"链接超时";
      errno = ETIMEDOUT;
      return -1;
    }
  }else{
    if(timer){
      timer->cancelTimer();
    }
    LOG_ERROR(logger) << "connect addEvent(" << fd << ", WRITE) error";
  }

  int err = 0;
  socklen_t errLen = sizeof(err);
  
  if(getsockopt(fd,SOL_SOCKET,SO_ERROR,&err,&errLen) == -1){
    return -1;
  }
  
  if(!err){
    return 0;
  }else{
    errno = err;
    return -1;
  }

}

int socket(int domain, int type, int protocol){
  LOG_INFO(logger) <<"ishooked";
  if(!gaiya::isHook()){
    return socket_f(domain,type,protocol);
  }
  int fd = socket_f(domain,type,protocol);

  if(fd == -1){
    return fd;
  }

  gaiya::fdMng::getInstance()->get(fd,true);
  return fd;
}

int close(int fd){
  if(!gaiya::isHook()){
    return close_f(fd);
  }
  gaiya::IOmanager* iom = gaiya::IOmanager::GetThis();
  gaiya::FdCtx::ptr ctx = gaiya::fdMng::getInstance()->get(fd);
  if(ctx){
    gaiya::fdMng::getInstance()->del(fd);
    iom->cancelAll(fd);
  }
  return close_f(fd);
}


int accept(int sockfd, struct sockaddr *addr,socklen_t *addrlen){
  int fd = do_io(sockfd,accept_f,"accept",gaiya::IOmanager::READ,SO_RCVTIMEO,addr,addrlen);
  if(fd >= 0){
    gaiya::fdMng::getInstance()->get(fd,true);
  }
  return fd;
}


int connect(int sockfd, const struct sockaddr *addr,socklen_t addrlen){
  return connect_with_timeout(sockfd,addr,addrlen,gaiya::s_timeout);
}

int fcntl(int fd, int cmd, ... /* arg */ ){
  va_list va;
  va_start(va,cmd);
  switch(cmd)
  {
  case F_SETFL: {
    int op = va_arg(va,int);
    va_end(va);
    gaiya::FdCtx::ptr ctx = gaiya::fdMng::getInstance()->get(fd);
    if(!ctx || ctx->isClose() || !ctx->isSocket()){
      return fcntl_f(fd,cmd,op);
    }
    if(op & O_NONBLOCK){
      ctx->SetUserNonBlock(true);
    }
    op |= ctx->getSysNonBlock() ? O_NONBLOCK : 0;
    return fcntl_f(fd,cmd,op);
    }
    break;
  case F_GETFL:{
    va_end(va);
    int op = fcntl_f(fd,cmd,0);
    gaiya::FdCtx::ptr ctx = gaiya::fdMng::getInstance()->get(fd);
    if(!ctx || ctx->isClose() || !ctx->isSocket()){
      return op;
    }
    op |= ctx->getSysNonBlock() ? O_NONBLOCK : 0;
    return op;
    }
    break;
    case F_DUPFD: //复制一个文件描述符。需要一个参数，指定要复制到的文件描述符编号。
    case F_DUPFD_CLOEXEC: //设置或清除close-on-exec标志。如果FD_CLOEXEC位是0，执行execve的过程中，文件保持打开。反之则关闭。
    case F_SETFD://设置文件描述符的标志。需要一个参数，指定要设置的标志
    case F_SETOWN: //设置进程或线程接收到信号的文件描述符。不需要额外参数
    case F_SETSIG://设置文件描述符上的信号号。需要一个参数，指定信号号。

    /*
    F_SETLEASE:
    这个命令用于建立或删除文件租约。文件租约提供了一个机制，当一个进程（称为“租约持有者”）试图打开或截断文件时，如果该文件已经被另一个进程（称为“租约破坏者”）设置了租约，系统会通知租约持有者。
    */
    case F_SETLEASE:
    case F_NOTIFY: 
#ifdef F_SETPIPE_SZ
    case F_SETPIPE_SZ:
#endif
    { //请求通知，当文件状态改变时。需要一个参数，指定通知选项。
      int arg = va_arg(va, int);
      va_end(va);
      return fcntl_f(fd, cmd, arg); 
    }
    break;
    case F_GETSIG:
    case F_GETOWN:
    case F_GETFD:
    //获取文件租约，当进程要打开文件时，会通知文件租约拥有者
    case F_GETLEASE:

#ifdef F_GETPIPE_SZ
    case F_GETPIPE_SZ:
#endif
    {
      va_end(va);
      return fcntl_f(fd, cmd); 
    }
    break;
    case F_GETLK:
    case F_SETLKW:
    case F_SETLK:{
      struct flock* arg = va_arg(va,struct flock*);
      va_end(va);
      return fcntl_f(fd, cmd,arg);
    }
    break;
    case F_GETOWN_EX:
    case F_SETOWN_EX:{
      struct f_owner_ex* arg = va_arg(va,struct f_owner_ex*);
      va_end(va);
      return fcntl_f(fd,cmd,arg);
    }
    default:{
      va_end(va);
      return fcntl_f(fd, cmd); 
    }
  }
}

int ioctl(int fd, unsigned long int request, ...){
  va_list va;
  va_start(va,request);
  void * arg = va_arg(va,void*);
  va_end(va);

  if(request == FIONBIO){
    gaiya::FdCtx::ptr fdCtx = gaiya::fdMng::getInstance()->get(fd);
    if(!fdCtx || fdCtx->isClose() || !fdCtx->isSocket()){
      return ioctl_f(fd,request,arg);
    }
    int ok = !!*((int*)arg);
    fdCtx->SetUserNonBlock(ok);
  }
  return ioctl_f(fd,request,arg);
}

int getsockopt(int sockfd, int level, int optname, void *optval, socklen_t *optlen){
  return getsockopt_f(sockfd,level,optname,optval,optlen);
}

int setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen){
  if(!gaiya::isHook()){
    return setsockopt_f(sockfd,level,optname,optval,optlen);
  }

  if(level == SOL_SOCKET){
    if(optname == SO_RCVTIMEO || optname == SO_SNDTIMEO){
      gaiya::FdCtx::ptr ctx = gaiya::fdMng::getInstance()->get(sockfd);
      if(ctx){
        struct timeval* val = (struct timeval*) optval;
        ctx->setTimeout(optname,val->tv_sec * 1000 + val->tv_usec / 1000);
      }
    }
  }
  return setsockopt_f(sockfd,level,optname,optval,optlen);
}


} 
