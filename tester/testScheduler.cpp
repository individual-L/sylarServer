#include"head.hpp"
static gaiya::Logger::ptr logger = LOG_GET_LOGGER("master");

void func1(){
  static int s_count = 5;
  LOG_INFO(logger) << "test coroutine func1 enter "<< s_count <<" time;";
  // gaiya::Thread::Sleep_for(2);
  if(--s_count >= 0){
    gaiya::Scheduler::GetThis()->schedule(&func1,gaiya::Thread::GetThis()->getId());
  }

}

int main(){
  gaiya::Config::loadYamlFile("/home/luo/cplus/gaiya/tester/configTest.yaml");
  gaiya::Thread::SetName("main");
  LOG_INFO(logger) <<"test start";
  gaiya::Scheduler::ptr scheduler(new gaiya::Scheduler(3,false,"test"));
  scheduler->start();
  scheduler->schedule(&func1);
  scheduler->stop();
  LOG_INFO(logger) <<"test end";
  return 1;
}
