#include"schedule.hpp"

namespace gaiya{
//当前的协程调度器指针
static thread_local Scheduler* t_scheduler = nullptr; 
//调度协程指针 外部有shared_ptr实例，故不需要线程管理,
static thread_local Coroutine* t_masterCoro = nullptr; 
//日志器
static gaiya::Logger::ptr logger = LOG_GET_LOGGER("master");

Scheduler::Scheduler(uint64_t size,bool useSche,const std::string& name):m_name(name){
  GAIYA_ASSERT(size > 0)
  //是否使用调度协程
  if(useSche){
    //创建主协程
    gaiya::Coroutine::GetCurCoro();
    //调度协程的线程占用一个名额
    size -= 1;

    //确保一个线程只有一个调度器
    GAIYA_ASSERT(t_scheduler == nullptr);
    t_scheduler = this;

    m_masterCoro.reset(new gaiya::Coroutine(bind(Scheduler::run,this)),0,true);
    gaiya::Thread::SetName(m_name);

    t_masterCoro = m_masterCoro.get();
    m_masterTid = gaiya::GetThreadId();
    m_threadIds.push_back(m_masterTid);
  }else{
    m_masterTid = -1;
  }

  m_threadCount = size;
}
Scheduler::~Scheduler(){
  if(GetThis().get() == this){
    t_scheduler = nullptr;
  }
}
Scheduler::ptr Scheduler::GetThis(){
  if(t_scheduler){
    return t_scheduler->shared_from_this();
  }
  return nullptr;
}

void Scheduler::setThis(){
  t_scheduler = this;
}

gaiya::Coroutine::ptr Scheduler::GetMasterCoro(){
  return t_masterCoro->shared_from_this();
}
void Scheduler::start(){
  MuteType::Lock lock(m_mutex);
  if(m_active){
    return;
  }
  m_active = true;
  GAIYA_ASSERT(m_tpool.empty());

  m_tpool.resize(m_threadCount);
  for(int i = 0;i < m_threadCount;++i){
    m_tpool[i].reset(new Thread(std::bind(&Scheduler::run,this)
    ,m_name + "_" + std::to_string(i)));

    m_threadIds.push_back(m_tpool[i]->getId());
  }
  
}

void Scheduler::stop(){
  m_autoStop = true;
  //
  if(m_masterCoro 
   && m_threadCount == 0
   && (m_masterCoro->getState() == Coroutine::INIT ||
        m_masterCoro->getState() == Coroutine::END)){
    LOG_INFO(logger) << "scheduler: " << m_name <<"stoped";
    if(stopping()){
      return;
    }
  }

  //是否使用调度协程，使用就必须由创建调度协程的调度器来关闭
  if(m_masterTid != -1){
    GAIYA_ASSERT(GetThis().get() == this);
  }else{
    GAIYA_ASSERT(GetThis().get() != this);
  }

  for(int i = 0;i < m_threadCount;++i){
    tickle();
  }

  std::vector<gaiya::Thread::ptr> tv;
  {
    MuteType::Lock lock(m_mutex);
    tv.swap(m_tpool);
  }

  //线程池中还有线程没有执行完，让主线程等待其执行完
  for(size_t i = 0; i < tv.size();++i){
    tv[i]->join();
  }

}
void Scheduler::tickle(){
  LOG_INFO(logger) << "tickle";
}

void Scheduler::run(){
  LOG_INFO(logger) <<"m_name: " << m_name << "running";
  setThis();

  if(gaiya::GetThreadId() != m_masterTid){
    t_masterCoro = Coroutine::GetCurCoro().get();
  }

  Coroutine::ptr idleCoro(new Coroutine(std::bind(idle,this)));
  Coroutine::ptr coro;
  CorFType corf;
  while(true){
    //
    bool tickle_me = false;
    //此线程是否在执行协程
    bool is_active = false;
    {
      MuteType::Lock lock(m_mutex);
      auto it = m_coros.begin();
      while(it != m_coros.end()){
      //协程指定线程是否由调度线程执行
      if(it->m_threadId != -1 && it->m_threadId == GetThis()->m_masterTid){
        ++it;
        tickle_me = true;
        continue;
      }
      GAIYA_ASSERT(it->m_coro || it->m_func);
      if(it->m_coro && it->m_coro->getState() == Coroutine::EXECU){
        ++it;
        continue;
      }
      corf = *it;
      m_coros.erase(it++);
      ++m_activeThreadCount;
      is_active = true;
      break;
      }
    }
          
    if(tickle_me){
      tickle();
    }
    if(corf.m_coro && (corf.m_coro->getState() != Coroutine::END 
                    && corf.m_coro->getState() != Coroutine::EXCEPT)){
      corf.m_coro->swapIn();
      --m_activeThreadCount;

      if(corf.m_coro->getState() == Coroutine::READY){
        schedule(corf.m_coro);
      }else if(corf.m_coro->getState() != Coroutine::END
            &&corf.m_coro->getState() != Coroutine::EXCEPT){
          corf.m_coro->m_state = Coroutine::HOLD;
      }
      corf.reset();
    }else if(corf.m_func){
      if(coro){
        coro->reset(corf.m_func);
      }else{
        coro.reset(new Coroutine(corf.m_func));
      }
      corf.reset();  
      coro->swapIn();
      --m_activeThreadCount;

      if(corf.m_coro->getState() == Coroutine::READY){
        schedule(coro);
        coro.reset();
      }else if(corf.m_coro->getState() != Coroutine::END
            &&corf.m_coro->getState() != Coroutine::EXCEPT){
          coro->m_state = Coroutine::HOLD;
      }
      //....
    }else{
      if(is_active){
        --m_activeThreadCount;
      }

      //进入idle协程
      ++m_idleThreadCount;
      idleCoro->swapIn();
      --m_idleThreadCount;

    }
  }

}

void Scheduler::idle(){

}

bool Scheduler::stopping(){

}
}
