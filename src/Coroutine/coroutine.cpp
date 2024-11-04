#include"coroutine.hpp"

namespace gaiya{
static gaiya::Logger::ptr logger = LOG_GET_LOGGER("master");

static std::atomic<uint64_t> s_coroutine_id = 0;
static std::atomic<uint64_t> s_coroutine_count = 0;

//每个线程拥有的主协程的指针
static thread_local gaiya::Coroutine::ptr s_threadCoroutine_ptr = nullptr;
//每个线程当前正在执行的协程指针
static thread_local gaiya::Coroutine* s_curCoroutine = nullptr;

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
  SetCur(this);

  if(::getcontext(&m_context)){
    GAIYA_ASSERT2(false,"getcontext");
  }
  m_id = 0;
  s_coroutine_count += 1;

  LOG_INFO(logger) << "Coroutine() coroutine id: " << m_id;

}


Coroutine::Coroutine(std::function<void()> func,size_t stacksize)
:m_id(++s_coroutine_id)
,m_func(func)
{
  s_coroutine_count += 1;
  m_stackSize = stacksize ? stacksize : config_stackSize->getValue();

  m_stack = StackAllocator::Alloc(m_stackSize);
  if(::getcontext(&m_context)){
    GAIYA_ASSERT2(false,"getcontext");
  }
  //此协程执行完自动指向主协程上下文
  m_context.uc_link = &s_threadCoroutine_ptr->m_context;
  m_context.uc_stack.ss_sp = m_stack;
  m_context.uc_stack.ss_size = m_stackSize;


  ::makecontext(&m_context,&Coroutine::MainFunc,0);

  LOG_INFO(logger) << "Coroutine() Coroutine::m_id: " << m_id <<" coroutine count: " <<s_coroutine_count;
}
Coroutine::~Coroutine(){
  s_coroutine_count -= 1;

  if(m_stack){
    GAIYA_ASSERT(m_state == INIT
      || m_state == EXCEPT
      || m_state == END);
    StackAllocator::Dealloc(m_stack);
  }else{
    GAIYA_ASSERT(m_state == EXECU);
    GAIYA_ASSERT(!m_func);

    //判断是否为最后一个执行的协程
    Coroutine* cur = s_curCoroutine;
    if(cur == this){
      s_curCoroutine = nullptr;
    }

  }
  LOG_INFO(logger) << "Coroutine::~Coroutine() id: " << m_id << " total: " << s_coroutine_count;

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
  SetCur(this);
  GAIYA_ASSERT(m_state != EXECU);
  m_state = EXECU;
  if(::swapcontext(&(s_threadCoroutine_ptr->m_context),&m_context)){
    GAIYA_ASSERT2(false,"swapcontext");
  }
}

//让出给主协程
void Coroutine::swapOut(){
  SetCur(s_threadCoroutine_ptr.get());

  if(::swapcontext(&m_context,&(s_threadCoroutine_ptr->m_context))){
      GAIYA_ASSERT2(false,"swapcontext");
  }

}

void Coroutine::SetCur(Coroutine * ptr){
  s_curCoroutine = ptr;
}

Coroutine::ptr Coroutine::GetCur(){
  if(s_curCoroutine){
    return s_curCoroutine->shared_from_this();
  }
  Coroutine::ptr main_coroutine(new Coroutine);
  GAIYA_ASSERT(s_curCoroutine == main_coroutine.get());

  s_threadCoroutine_ptr = main_coroutine;
  main_coroutine.reset();
  return s_curCoroutine->shared_from_this();
}

void Coroutine::YieldToReady(){
  Coroutine::ptr cur = GetCur();
  GAIYA_ASSERT(cur->m_state == EXECU);

  cur->m_state = READY;
  cur->swapOut();
  
}

void Coroutine::YieldToHold(){
  Coroutine::ptr cur = GetCur();
  GAIYA_ASSERT(cur->m_state == EXECU);

  cur->m_state = HOLD;
  cur->swapOut();

}

uint64_t Coroutine::GetCoroutineCount(){
  return s_coroutine_count;
}

uint64_t Coroutine::GetCurId(){
  if(s_curCoroutine){
    return s_curCoroutine->getId();
  }
  return 0;
}

void Coroutine::MainFunc(){
  Coroutine::ptr cur = GetCur();
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
