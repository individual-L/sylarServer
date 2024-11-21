#include"thread.hpp"
#include<thread>

namespace gaiya{

//线程局部变量
static thread_local std::string s_threadName = "UNKONWN";
static thread_local Thread* s_thread = nullptr;

static gaiya::Logger::ptr s_logger = LOG_GET_LOGGER("master");

Thread::Thread(const std::function<void()> cb,const std::string name)
:m_name(name),m_cb(cb){
  if(name.empty()){
    m_name = "UNKONWN";
  }
  //返回0为成功
  int res = pthread_create(&m_thread,nullptr,&Thread::run,this);
  if(res){
    LOG_ERROR(s_logger) << "pthread_create fail res = " <<res <<"threadName = " << name;
    throw std::logic_error("pthread_create fail"); 
  }
  
  //等待线程局部变量初始化再返回
  m_sem.wait();
  // LOG_INFO(s_logger) <<"thread id: " <<m_id << " initiated ";
}

Thread::~Thread(){
  if(m_thread){
    int res = pthread_detach(m_thread);
    if(res){
      LOG_INFO(s_logger) << "pthread_detach fail res = " <<res;
    }
  }
}



//线程执行函数
void* Thread::run(void * arg ){
  Thread* thread = reinterpret_cast<Thread*>(arg);

  s_thread = thread;

  s_threadName = thread->getName();

  thread->m_id = gaiya::GetThreadId();

  pthread_setname_np(pthread_self(),thread->getName().substr(0,15).c_str());

  //使用临时变量接收函数并执行，执行完成后会自动析构，这样这个线程执行完成后可继续传递函数给此线程
  std::function<void()> cb;
  //防止函数内有智能指针，如果用等于号会增加智能指针的引用次数，此步骤可以减少引用
  cb.swap(thread->m_cb);

  thread->m_sem.post();
  
  cb();

  return 0;
}
//设置当前线程名称
void Thread::SetName(std::string name){
  if(name.empty()){
    return;
  }
  if(s_thread){
    s_thread->setName(name);
  }
  s_threadName = name;
}

//获取当前线程名称
std::string Thread::GetName(){
  return s_threadName;
}

//调用此函数的线程等待线程执行完成
void Thread::join(){
  if(m_thread){
    /*
    第二个参数 void **retval 是一个指向 void 指针的指针，用于存储线程的返回值。
    会阻塞调用线程，直到m_thread线程执行完成
    */
    int res = pthread_join(m_thread,nullptr);
    if(res){
      LOG_ERROR(s_logger) <<"pthread_join fail res = " << res <<" name = " << m_name;
      throw std::logic_error("pthread_join error");
    }
    m_thread = 0;
  }
}
//获取当前线程
Thread* Thread::GetThis(){
  if(s_thread){
    return s_thread;
  }
  return nullptr;
}
void Thread::Sleep_for(uint64_t times){
  std::this_thread::sleep_for(std::chrono::seconds(times));
}


}
