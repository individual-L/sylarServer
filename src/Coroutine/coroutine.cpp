#include"coroutine.hpp"

namespace gaiya{
static gaiya::Logger::ptr logger = LOG_GET_LOGGER("master");

//为了避免id相同，此非线程独有
static std::atomic<uint64_t> s_coro_id = 0;
static std::atomic<uint64_t> s_coro_count = 0;

//每个线程拥有的主协程的指针,外部没有，故设置为智能指针对象
static thread_local gaiya::Coroutine::ptr t_threadCoro = nullptr;
//每个线程当前正在执行的协程指针，通常外部存在一个智能指针对象，可以通过shared_from_this()获取，所有不需要在设置一个智能指针对象
static thread_local gaiya::Coroutine* t_curCoro = nullptr;

gaiya::ConfigVar<uint32_t>::ptr config_stackSize = gaiya::Config::lookup<uint32_t>("coroutine.stacksize","coroutine stack size",1024 * 128);

class StackAllocator{
  public:
    static void* Alloc(const size_t size){
      return malloc(size);
    };
    static void Dealloc(void* ptr){
      free(ptr);
    };
};
//线程的主协程
Coroutine::Coroutine()
{
  m_state = EXECU;
  SetCurCoro(this);

  if(::getcontext(&m_context)){
    GAIYA_ASSERT2(false,"getcontext");
  }
  m_id = 0;
  s_coro_count += 1;

  LOG_INFO(logger) << "Coroutine() coroutine id: " << m_id;

}


Coroutine::Coroutine(std::function<void()> func,size_t stacksize,bool retnSche)
:m_id(++s_coro_id)
,m_func(func)
{
  s_coro_count += 1;
  m_stackSize = stacksize ? stacksize : config_stackSize->getValue();

  m_stack = StackAllocator::Alloc(m_stackSize);
  if(::getcontext(&m_context)){
    GAIYA_ASSERT2(false,"getcontext");
  }
  //此协程执行完自动指向主协程上下文
  
  m_context.uc_stack.ss_sp = m_stack;
  m_context.uc_stack.ss_size = m_stackSize;

  //协程结束之后返回到调度器还是此线程的主协程
  if(retnSche){
    m_context.uc_link = &gaiya::Scheduler::GetMasterCoro()->m_context;
  }else{
    m_context.uc_link = &t_threadCoro->m_context;
  }
  ::makecontext(&m_context,&Coroutine::MainFunc,0);

  LOG_INFO(logger) << "Coroutine() Coroutine::m_id: " << m_id <<" coroutine count: " <<s_coro_count;
}
Coroutine::~Coroutine(){
  s_coro_count -= 1;

  if(m_stack){
    GAIYA_ASSERT(m_state == INIT
      || m_state == EXCEPT
      || m_state == END);
    StackAllocator::Dealloc(m_stack);
  }else{
    GAIYA_ASSERT(m_state == EXECU);
    GAIYA_ASSERT(!m_func);

    //判断是否为最后一个执行的协程
    Coroutine* cur = t_curCoro;
    if(cur == this){
      t_curCoro = nullptr;
    }

  }
  LOG_INFO(logger) << "Coroutine::~Coroutine() id: " << m_id << " total: " << s_coro_count;

}

void Coroutine::reset(std::function<void()> func){
  GAIYA_ASSERT(m_state == INIT
    || m_state == END
    || m_state == EXCEPT);
  GAIYA_ASSERT(m_stack);


  m_func.swap(func);

  if(::getcontext(&m_context)){
    GAIYA_ASSERT2(false,"getcontext");
  }

  m_context.uc_link = nullptr;
  m_context.uc_stack.ss_sp = m_stack;
  m_context.uc_stack.ss_size = m_stackSize;

  ::makecontext(&m_context,Coroutine::MainFunc ,0);

  m_state = INIT;
}

void Coroutine::swapIn(){
  SetCurCoro(this);
  GAIYA_ASSERT(m_state != EXECU);
  m_state = EXECU;
  if(::swapcontext(&(gaiya::Scheduler::GetMasterCoro()->m_context),&m_context)){
    GAIYA_ASSERT2(false,"swapcontext");
  }
}

//让出给主协程
void Coroutine::swapOut(){
  SetCurCoro(gaiya::Scheduler::GetMasterCoro().get());

  if(::swapcontext(&m_context,&(gaiya::Scheduler::GetMasterCoro()->m_context))){
      GAIYA_ASSERT2(false,"swapcontext");
  }

}

void Coroutine::call(){
  SetCurCoro(this);
  GAIYA_ASSERT(m_state != EXECU);
  m_state = EXECU;
  if(::swapcontext(&(t_threadCoro->m_context),&m_context)){
    GAIYA_ASSERT2(false,"swapcontext");
  }
}

//让出给主协程
void Coroutine::back(){
  SetCurCoro(t_threadCoro.get());

  if(::swapcontext(&m_context,&(t_threadCoro->m_context))){
      GAIYA_ASSERT2(false,"swapcontext");
  }

}

void Coroutine::SetCurCoro(Coroutine * ptr){
  t_curCoro = ptr;
}

Coroutine::ptr Coroutine::GetCurCoro(){
  if(t_curCoro){
    return t_curCoro->shared_from_this();
  }
  //创建0号协程，即主协程
  Coroutine::ptr main_coroutine(new Coroutine);
  GAIYA_ASSERT(t_curCoro == main_coroutine.get());

  t_threadCoro = main_coroutine;
  main_coroutine.reset();
  return t_curCoro->shared_from_this();
}

void Coroutine::YieldToReady(){
  Coroutine::ptr cur = GetCurCoro();
  GAIYA_ASSERT(cur->m_state == EXECU);

  cur->m_state = READY;
  cur->swapOut();
  
}

void Coroutine::YieldToHold(){
  Coroutine::ptr cur = GetCurCoro();
  GAIYA_ASSERT(cur->m_state == EXECU);

  cur->m_state = HOLD;
  cur->swapOut();

}

uint64_t Coroutine::GetCoroCount(){
  return s_coro_count;
}

uint64_t Coroutine::GetCurCoroId(){
  if(t_curCoro){
    return t_curCoro->getId();
  }
  return 0;
}

void Coroutine::MainFunc(){
  Coroutine::ptr cur = GetCurCoro();
  try{
    cur->m_func();
    cur->m_func = nullptr;
    cur->m_state = END;
  }catch(std::exception& ex){
    cur->m_state = EXCEPT;
    LOG_ERROR(logger) <<"Coroutine::MainFunc() error:" << ex.what() 
    << "coroutine id: " << cur->getId()
    <<std::endl
    <<backTraceToString();
  }catch(...){
    cur->m_state = EXCEPT;
    LOG_ERROR(logger) <<"Coroutine::MainFunc() error"
    << "coroutine id: " << cur->getId()
    <<std::endl
    <<backTraceToString();
  }
  // Coroutine * raw_ptr = cur.get();
  // cur.reset();
  // raw_ptr->swapOut();

}
}
