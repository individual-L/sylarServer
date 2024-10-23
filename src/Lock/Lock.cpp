#include"lock.hpp"


namespace gaiya{
  /*
    参数说明：
    pshared：指定信号量是否可以被其他进程访问。如果设置为非零值，信号量可以被其他进程访问；如果设置为0，信号量只能被同一进程内的线程访问。
    成功时返回0。
    失败时返回-1，并设置errno以指示错误类型。
  */
Semaphore::Semaphore(uint32_t count = 0){
  if(sem_init(&m_semaphore,0,count)){
    throw std::logic_error("sem_init error");
  }
}
Semaphore::~Semaphore(){
  sem_destroy(&m_semaphore);
}

void Semaphore::wait(){
  if(sem_wait(&m_semaphore)){
    throw std::logic_error("sem_wait error");
  }
}

void Semaphore::post(){
  if(sem_post(&m_semaphore)){
    throw std::logic_error("sem_post error");
  }
  
}







  
}
