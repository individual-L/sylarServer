#include"schedule.hpp"
#include"macro.hpp"
#include"log.hpp"
#include"util.hpp"
#include"hook.hpp"

namespace gaiya{
//当前的协程调度器指针
static thread_local Scheduler* t_scheduler = nullptr; 
//指向正在运行的调度协程的指针 外部有shared_ptr实例，故不需要线程管理,
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
    //如果是子类调用的父类构造函数，那么this指针指向的是子类；否则指向的是正在构造的父类对象
    t_scheduler = this;

    m_masterCoro.reset(new gaiya::Coroutine(std::bind(&Scheduler::run,this),0,false));
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
  if(t_scheduler && t_scheduler == this){
    t_scheduler = nullptr;
  }
}
Scheduler* Scheduler::GetThis(){
  if(t_scheduler){
    return t_scheduler;
  }
  return nullptr;
}

void Scheduler::setThis(){
  t_scheduler = this;
}

gaiya::Coroutine* Scheduler::GetMasterCoro(){
  return t_masterCoro;
}
void Scheduler::start(){
  {
    MuteType::Lock lock(m_mutex);
    if(!m_stopping){
      return;
    }
    m_stopping = false;
    GAIYA_ASSERT(m_tpool.empty());

    m_tpool.resize(m_threadCount);
    // LOG_INFO(logger) <<"size: " << m_threadCount;
    for(uint32_t i = 0;i < m_threadCount;++i){
      m_tpool[i].reset(new Thread(std::bind(&Scheduler::run,this)
      ,m_name + "_" + std::to_string(i + 1)));

      m_threadIds.push_back(m_tpool[i]->getId());
    }
  }
  // if(t_masterCoro){
  //   LOG_INFO(logger) << "masterCoroing...";
  //   t_masterCoro->call();
  // }
}

void Scheduler::stop(){
  // LOG_INFO(logger) << "scheduler stopping";
  m_autoStop = true;
  if(m_masterCoro 
   && m_threadCount == 0        //只有调度线程
   && (m_masterCoro->getState() == Coroutine::INIT ||
        m_masterCoro->getState() == Coroutine::END)){
    LOG_INFO(logger) << "scheduler: " << m_name <<" can stop";

    m_stopping = true;
    if(stopping()){
      return;
    }
  }

  //是否使用调度协程，使用就必须由创建调度协程的调度器来关闭
  if(m_masterTid != -1){
    GAIYA_ASSERT(GetThis() == this);
  }else{
    GAIYA_ASSERT(GetThis() != this);
  }

  m_stopping = true;
  for(uint32_t i = 0;i < m_threadCount;++i){
    tickle();
  }
  //启动调度协程
  if(m_masterCoro){
    if(!stopping()){
      LOG_INFO(logger) << "masterCoroing...";
      m_masterCoro->call();
    }else{
      LOG_INFO(logger) << "调度协程不执行";
    }
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
  setThis();
  gaiya::setThreadHook(true);
  if(gaiya::GetThreadId() != m_masterTid){
    t_masterCoro = Coroutine::GetCurCoro().get();
  }
  Coroutine::ptr idleCoro(new Coroutine(std::bind(&Scheduler::idle,this),0,true));
  LOG_INFO(logger)<<"idleCoro id: " <<idleCoro->getId();
  Coroutine::ptr coro;
  CorFType corf;

  while(true){
    //
    bool tickle_me = false;
    //此线程是否获取到任务需要执行
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
      // LOG_INFO(logger) <<" get mission";
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
        //如果没有执行完或者协程让出来执行权，设置为hold
      }else if(corf.m_coro->getState() != Coroutine::END
            &&corf.m_coro->getState() != Coroutine::EXCEPT){
          corf.m_coro->m_state = Coroutine::HOLD;
      }
      corf.reset();
    }else if(corf.m_func){
      if(coro){
        coro->reset(corf.m_func);
      }else{
        coro.reset(new Coroutine(corf.m_func,0,true));
      }
      corf.reset();  
      coro->swapIn();
      --m_activeThreadCount;

      if(coro->getState() == Coroutine::READY){
        schedule(coro);
        coro.reset();
      }else if(coro->getState() == Coroutine::END
            || coro->getState() == Coroutine::EXCEPT){
          coro->reset(nullptr);
      }else{
          coro->m_state = Coroutine::HOLD;
          coro.reset();
      }
    }else{
      if(is_active){
        --m_activeThreadCount;
      }
      //idle协程执行完
      if(idleCoro->getState() == Coroutine::END){
        LOG_INFO(logger) << "idle finished";
        break;
      }
      //进入idle协程
      ++m_idleThreadCount;
      // LOG_INFO(logger) <<"enter idleCoro id: " <<idleCoro->getId();
      idleCoro->swapIn();
      --m_idleThreadCount;
      // if(idleCoro->getState() != gaiya::Coroutine::END 
      //   && idleCoro->getState() != gaiya::Coroutine::EXCEPT){
      //     idleCoro->m_state = gaiya::Coroutine::HOLD;
      // }

    }
  }

}

void Scheduler::idle(){
  LOG_INFO(logger) <<"idle start";
  while(!stopping()){
    gaiya::Coroutine::YieldToHold();
  }
}

bool Scheduler::stopping(){
  MuteType::Lock lock(m_mutex);
  // LOG_INFO(logger) <<"m_autoStop: " <<m_autoStop
  //                   <<" m_stopping: " <<m_stopping
  //                   <<" m_coros.empty: " <<m_coros.empty()
  //                   <<" m_activeThreadCount: " <<(m_activeThreadCount == 0);
  return m_autoStop && m_stopping
        && m_coros.empty() && m_activeThreadCount == 0;
}


}
