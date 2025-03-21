#include"iomanager.hpp"
#include"log.hpp"
#include"macro.hpp"

#include<sys/epoll.h>
#include<fcntl.h> //file control
#include<errno.h>
#include<memory>


namespace gaiya{

static gaiya::Logger::ptr logger = LOG_GET_LOGGER("master");

enum EPCTLOP{

};

static std::ostream& operator <<(std::ostream& os, const EPOLL_EVENTS& events){
  if(!events){
    os <<"0";
    return os;
  }
bool first = true;
#define XX(E) \
  if(E & events){ \
    if(!first){ \
      os << " | "; \
    } \
    os << #E; \
    first = false; \
  } 
  XX(EPOLL_EVENTS::EPOLLIN);
  XX(EPOLL_EVENTS::EPOLLPRI);
  XX(EPOLL_EVENTS::EPOLLOUT);
  XX(EPOLL_EVENTS::EPOLLRDNORM);
  XX(EPOLL_EVENTS::EPOLLRDBAND);
  XX(EPOLL_EVENTS::EPOLLWRNORM);
  XX(EPOLL_EVENTS::EPOLLWRBAND);
  XX(EPOLL_EVENTS::EPOLLMSG);
  XX(EPOLL_EVENTS::EPOLLERR);
  XX(EPOLL_EVENTS::EPOLLHUP);
  XX(EPOLL_EVENTS::EPOLLRDHUP);
  XX(EPOLL_EVENTS::EPOLLEXCLUSIVE);
  XX(EPOLL_EVENTS::EPOLLONESHOT);
  XX(EPOLL_EVENTS::EPOLLET);

#undef XX
  return os;
}

static std::ostream& operator <<(std::ostream& os, const IOmanager::EventType& events){
  if(!events){
    os <<"0";
    return os;
  }
bool first = true;
#define XX(E) \
  if(E & events){ \
    if(!first){ \
      os << " | "; \
    } \
    os << #E; \
    first = false; \
  } 
  XX(EPOLL_EVENTS::EPOLLIN);
  XX(EPOLL_EVENTS::EPOLLPRI);
  XX(EPOLL_EVENTS::EPOLLOUT);
  XX(EPOLL_EVENTS::EPOLLRDNORM);
  XX(EPOLL_EVENTS::EPOLLRDBAND);
  XX(EPOLL_EVENTS::EPOLLWRNORM);
  XX(EPOLL_EVENTS::EPOLLWRBAND);
  XX(EPOLL_EVENTS::EPOLLMSG);
  XX(EPOLL_EVENTS::EPOLLERR);
  XX(EPOLL_EVENTS::EPOLLHUP);
  XX(EPOLL_EVENTS::EPOLLRDHUP);
  XX(EPOLL_EVENTS::EPOLLEXCLUSIVE);
  XX(EPOLL_EVENTS::EPOLLONESHOT);
  XX(EPOLL_EVENTS::EPOLLET);

#undef XX
  return os;
}

static std::ostream& operator <<(std::ostream& os, const EPCTLOP& op){
  if(!op){
    os <<"0";
    return os;
  }
  switch ((int)op)
  {
#define XX(E) \
  case E: \
    os << #E; \
    break;
  
  XX(EPOLL_CTL_ADD);
  XX(EPOLL_CTL_DEL);
  XX(EPOLL_CTL_MOD);
#undef XX

  default:
    os <<"UNKNOWN";
    break;
  }

  return os;
}


IOmanager::IOmanager(uint32_t threadSize,bool useSche,const std::string& name)
:Scheduler(threadSize,useSche,name){
  m_epfd = ::epoll_create(5000);
  GAIYA_ASSERT(m_epfd > 0);

  int res = pipe(m_tickleFds);
  GAIYA_ASSERT(!res)

  epoll_event event;
  memset(&event,0,sizeof(epoll_event));
  event.events = EPOLLIN | EPOLLET;
  event.data.fd = m_tickleFds[0];

  //设置此文件描述符为非阻塞模式
  res = fcntl(m_tickleFds[0],F_SETFL,O_NONBLOCK);
  GAIYA_ASSERT(!res);

  res = epoll_ctl(m_epfd,EPOLL_CTL_ADD,m_tickleFds[0],&event);
  GAIYA_ASSERT(!res);

  contextResize(32);

  start();

}

IOmanager::~IOmanager(){
  stop();

  close(m_epfd);

  close(m_tickleFds[0]);
  close(m_tickleFds[1]);
  for(size_t i = 0;i < m_fdctxs.size();++i){
    if(m_fdctxs[i]){
      delete m_fdctxs[i];
    }
  }
}

bool IOmanager::addEvent(int fd,EventType event,std::function<void()> cb){
  FdContext * fd_ctx = nullptr;
  MutexType::ReadLock lock(m_mutex);
  //拿到fd的上下文
  if(m_fdctxs.size() > (size_t)fd){
    fd_ctx = m_fdctxs[fd];
    lock.unlock();
  }else{
    lock.unlock();
    MutexType::WriteLock lock2(m_mutex);
    contextResize(fd * 1.5);
    fd_ctx = m_fdctxs[fd];
  }

  FdContext::MutexType::Lock lock2(fd_ctx->mutex);
  if(GAIYA_UNLIKELY((event & fd_ctx->events))){
    LOG_ERROR(logger) << "addEvent() error, additional event is different from m_fdctxs[fd]->event "<<"additional event: " << (EPOLL_EVENTS)event
    <<"m_fdctxs[fd]->event: " <<fd_ctx->events;
    GAIYA_ASSERT(!(event & fd_ctx->events))
  }

  int op = fd_ctx->events ? EPOLL_CTL_MOD : EPOLL_CTL_ADD;

  epoll_event ev;
  ev.events = fd_ctx->events | event | EPOLLET;
  ev.data.ptr = fd_ctx;

  int res = epoll_ctl(m_epfd,op,fd,&ev);
  // LOG_INFO(logger)<<"fd: "<<fd<<" " << ((EventType)event);

  if(res){
    LOG_ERROR(logger) << "epoll_ctl(" << m_epfd <<"," << (EPCTLOP)op <<"," <<fd <<"," 
    << (EPOLL_EVENTS)ev.events <<");"
    << res <<"(" << errno <<") = (" << strerror(errno) <<") ctx->events = "
    << (EPOLL_EVENTS)fd_ctx->events;
    return false;
  }

  ++m_pendingEventCount;

  fd_ctx->events = (EventType)(fd_ctx->events | event);
  FdContext::EventContext& ev_ctx = fd_ctx->getContext(event);
  GAIYA_ASSERT(!ev_ctx.scheduler
              &&!ev_ctx.coro
              &&!ev_ctx.cb);

  ev_ctx.scheduler = gaiya::Scheduler::GetThis();  
  if(cb){
    ev_ctx.cb.swap(cb);
  }else{
    ev_ctx.coro = gaiya::Coroutine::GetCurCoro();
    // GAIYA_ASSERT2(ev_ctx.coro->getState() == gaiya::Coroutine::State::EXECU
    //               ,"state=" << ev_ctx.coro->getState());
  }
  
  return true;
}

bool IOmanager::delEvent(int fd,EventType event){
  MutexType::ReadLock lock(m_mutex);
  if(m_fdctxs.size() <= (size_t)fd){
    return false;
  }
  FdContext* fd_ctx = m_fdctxs[fd];
  lock.unlock();

  FdContext::MutexType::Lock lock2(fd_ctx->mutex);
  //检测fd句柄是否添加event事件
  if(GAIYA_UNLIKELY(!(event & fd_ctx->events))){
    LOG_ERROR(logger) << "IOmanager::delEvent(): event is different from fdctxs[fd]->event";
    GAIYA_ASSERT(!(event & fd_ctx->events));
  }

  //检测fd句柄是否拥有其他事件
  EventType remained_event = (EventType)(fd_ctx->events & ~event);
  int op = remained_event ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;

  epoll_event ev;
  memset(&ev,0,sizeof(epoll_event));
  ev.data.fd = fd;
  ev.events = EPOLLET | remained_event;
  int res = epoll_ctl(m_epfd,op,fd,&ev);
  LOG_INFO(logger) <<"del event: "<<event;

  if(res){
    LOG_ERROR(logger) << "epoll_ctl(" << m_epfd <<"," << (EPCTLOP)op <<"," <<fd <<"," 
    << (EPOLL_EVENTS)ev.events <<");"
    << res <<"(" << errno <<") = (" << strerror(errno) <<") ctx->events = "
    << (EPOLL_EVENTS)fd_ctx->events;
    return false;
  }

  --m_pendingEventCount;
  fd_ctx->events = remained_event;
  FdContext::EventContext ev_ctx = fd_ctx->getContext(event);
  fd_ctx->resetContext(ev_ctx);
  return true;
}

IOmanager* IOmanager::GetThis(){
  return dynamic_cast<IOmanager*>(Scheduler::GetThis());
}
void IOmanager::idle() {
  LOG_INFO(logger)<<"idle";
  const uint64_t MAX_EVENTS = 256;
  //epoll_event类型的数组，创建智能指针去管理它
  epoll_event* evs = new epoll_event[MAX_EVENTS]();

  std::shared_ptr<epoll_event> evs_sptr(evs,[](epoll_event* evs){
    delete [] evs;
  });

  while(true){
    uint64_t nextTime = getNextTime();
    if(GAIYA_UNLIKELY(stopping(nextTime))){
      LOG_INFO(logger) <<"name = " <<getName() <<" idle stoped";
      break;
    }

    int res = 0;
    do{
      static const int MAX_TIMEOUT = 12000;
      if(nextTime != ~0ull){
        nextTime = nextTime > MAX_TIMEOUT ? MAX_TIMEOUT : nextTime;
      }else{
        nextTime = MAX_TIMEOUT;
      }
      res = epoll_wait(m_epfd,evs,MAX_EVENTS,(int)nextTime);
      // LOG_INFO(logger) <<"epoll_wait res = " <<res;
      //返回0表示发生错误，如果是中断，那就继续执行wait
      if(res < 0 && errno == EINTR){
      }else{
        break;
      }

    }while(true);

    std::vector<std::function<void()>> cbs;
    TimerMng::getTriggerableCB(cbs);
    if(!cbs.empty()){
      // LOG_INFO(logger) <<"add timer cb";
      schedule(cbs.begin(),cbs.end());
      cbs.clear();
    }

    for(int i = 0;i < res;++i){
      //触发的事件
      epoll_event& event = evs[i];
      if(event.data.fd == m_tickleFds[0]){
        // LOG_INFO(logger) <<"pipe fd: " <<event.data.fd <<" tirggered " << fcntl(m_tickleFds[0],F_GETFD,0);
        uint8_t ss[256];
        while(read(m_tickleFds[0],ss,sizeof(ss)) > 0);
        continue;
      }
      // LOG_INFO(logger) <<"fd: " <<event.data.fd <<" tirggered ";
      FdContext* fd_ctx = (FdContext*)event.data.ptr;
      FdContext::MutexType::Lock lock(fd_ctx->mutex);
      
      int real_event = NONE;
      if(event.events & (EPOLLERR | EPOLLHUP)){
        // LOG_INFO(logger) <<"EPOLLERR | EPOLLHUP";
        event.events |= (READ | WRITE) & fd_ctx->events;
      }
      if(event.events & READ){
        real_event |= READ;
      }
      if(event.events & WRITE){
        real_event |= WRITE;
      }
      if((real_event & fd_ctx->events) == NONE){
        continue;
      }

      int remained_ev = (fd_ctx->events & ~event.events);
      int op = remained_ev ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
      event.events = remained_ev | EPOLLET;
      int res = epoll_ctl(m_epfd,op,fd_ctx->fd,&event);

      // LOG_INFO(logger)<<"fd: "<<fd_ctx->fd <<" del event to: "<<(EPOLL_EVENTS)remained_ev;
      if(res){
        LOG_ERROR(logger) <<"epoll_ctl(" <<m_epfd <<", "<<(EPCTLOP)op <<", " <<event.data.fd <<", "
                          << (EPOLL_EVENTS)event.events <<";" 
                          <<res <<"(" <<errno <<") = " <<strerror(errno);
        continue;
      }
      if(real_event & READ){
        fd_ctx->triggerEvent(READ);
        --m_pendingEventCount;
      }
      if(real_event & WRITE){
        fd_ctx->triggerEvent(WRITE);
        --m_pendingEventCount;
      }
      
    }
    gaiya::Coroutine::YieldToHold();
  }
}

void IOmanager::tickle() {
  // LOG_INFO(logger) <<"IOmanager::tickle()";
  if(!hasThread()){
    return;
  }
  int res = write(m_tickleFds[1],"1",1);
  GAIYA_ASSERT(res == 1);
}

void IOmanager::onTimersInsertedAtFront(){
  // LOG_INFO(logger) <<"onTimersInsertedAtFront";
  tickle();
}

bool IOmanager::stopping() {
  uint64_t nextTime = 0;
  return stopping(nextTime);
}
bool IOmanager::stopping(uint64_t& nextTime){
  nextTime = getNextTime();
  // LOG_INFO(logger)<<"pendEvent: " <<m_pendingEventCount 
  //                 <<" nextTime: " <<(nextTime == ~0ull)
  //                 <<" stopping: " <<Scheduler::stopping();
  return m_pendingEventCount == 0 && Scheduler::stopping()
        && nextTime == ~0ull;
}

bool IOmanager::cancelEvent(int fd, EventType event){
  MutexType::ReadLock lock(m_mutex);
  if(m_fdctxs.size() <= (size_t)fd){
    return false;
  }
  FdContext* fd_ctx = m_fdctxs[fd];
  lock.unlock();

  FdContext::MutexType::Lock lock2(fd_ctx->mutex);
  if(GAIYA_UNLIKELY(!(fd_ctx->events & event))){
    LOG_ERROR(logger) << "IOmanager::cancelEvent():" <<(EPOLL_EVENTS)event <<" is different from fdctxs[fd]->event: "<<(EPOLL_EVENTS)(fd_ctx->events);
    GAIYA_ASSERT(!(event & fd_ctx->events));
  }

  EventType remained_event = (EventType)(fd_ctx->events & ~event);
  int op = remained_event ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;

  epoll_event ev;
  memset(&ev,0,sizeof(epoll_event));
  ev.events = remained_event | EPOLLET;
  ev.data.fd = fd;
  ev.data.ptr = fd_ctx;

  int res = epoll_ctl(m_epfd,op,fd,&ev);

  // LOG_INFO(logger) <<"del event: "<<event;

  if(res){
    LOG_ERROR(logger) << "epoll_ctl(" << m_epfd <<"," << (EPCTLOP)op <<"," <<fd <<"," 
    << (EPOLL_EVENTS)ev.events <<");"
    << res <<"(" << errno <<") = (" << strerror(errno) <<") ctx->events = "
    << (EPOLL_EVENTS)fd_ctx->events;
    return false;
  }
  fd_ctx->triggerEvent(event);
  --m_pendingEventCount;
  return true;
}

bool IOmanager::cancelAll(int fd){
  MutexType::ReadLock lock(m_mutex);
  if(m_fdctxs.size() <= (size_t)fd){
    return false;
  }
  FdContext* fd_ctx = m_fdctxs[fd];
  lock.unlock();

  FdContext::MutexType::Lock lock2(fd_ctx->mutex);
  if(!fd_ctx->events){
    return false;
  }

  epoll_event ev;
  memset(&ev,0,sizeof(epoll_event));
  ev.events = 0;
  ev.data.ptr = fd_ctx;

  int res = epoll_ctl(m_epfd,EPOLL_CTL_DEL,fd,&ev);
  if(res){
    LOG_ERROR(logger) << "epoll_ctl(" << m_epfd <<"," << (EPCTLOP)EPOLL_CTL_DEL <<"," <<fd <<"," 
    << (EPOLL_EVENTS)ev.events <<");"
    << res <<"(" << errno <<") = (" << strerror(errno) <<") ctx->events = "
    << (EPOLL_EVENTS)fd_ctx->events;
    return false;
  }
  if(fd_ctx->events & READ){
    fd_ctx->triggerEvent(READ);
    --m_pendingEventCount;
  }
  if(fd_ctx->events & WRITE){
    fd_ctx->triggerEvent(WRITE);
    --m_pendingEventCount;
  }
  GAIYA_ASSERT(fd_ctx->events == 0);
  return true;
}

void IOmanager::contextResize(size_t size){
  m_fdctxs.resize(size);
  for(size_t i = 0; i < m_fdctxs.size(); ++i) {
    if(!m_fdctxs[i]) {
        m_fdctxs[i] = new FdContext;
        m_fdctxs[i]->fd = i;
    }
  }
}

IOmanager::FdContext::EventContext& IOmanager::FdContext::getContext(const EventType& event){
  switch (event)
  {
  case IOmanager::EventType::READ:
    return read;
  case IOmanager::EventType::WRITE:
    return write;
  default:
    GAIYA_ASSERT2(false,"getContext");
  }
  throw std::invalid_argument("IOmanager::FdContext::getContext error");
}

void IOmanager::FdContext::resetContext(EventContext& ctx){
  ctx.scheduler = nullptr;
  ctx.coro.reset();
  ctx.cb = nullptr;
}

void IOmanager::FdContext::triggerEvent(const EventType& event){
  GAIYA_ASSERT(events & event);

  events = (EventType)(events & ~event);
  EventContext& ev_ctx = getContext(event);
  if(ev_ctx.cb){
    ev_ctx.scheduler->schedule(&ev_ctx.cb);
  }else{
    ev_ctx.scheduler->schedule(&ev_ctx.coro);
  }
  ev_ctx.scheduler = nullptr;
}

}
