#include"schedule.hpp"

namespace gaiya{
//当前的协程调度器
static thread_local Scheduler::ptr s_scheduler = nullptr; 
//外部有shared_ptr实例，故不需要线程管理
static thread_local Coroutine* s_curcoro = nullptr; 

Scheduler::Scheduler(uint64_t size,const std::string& name){

}
Scheduler::~Scheduler(){

}
Scheduler::ptr Scheduler::GetThis(){
  if(s_scheduler){
    return s_scheduler;
  }
  return nullptr;
}
void Scheduler::start(){

}

void Scheduler::stop(){

}
void Scheduler::tickle(){

}

void Scheduler::run(){

}

void Scheduler::idle(){

}


}
