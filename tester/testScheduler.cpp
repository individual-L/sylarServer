#include"head.hpp"
static gaiya::Logger::ptr logger = LOG_GET_LOGGER("master");
int main(){
  gaiya::Config::loadYamlFile("/home/luo/cplus/gaiya/tester/configTest.yaml");
  gaiya::Thread::SetName("main");
  LOG_INFO(logger) <<"test start";
  gaiya::Scheduler::ptr scheduler(new gaiya::Scheduler(2));
  scheduler->start();

  scheduler->stop();
  LOG_INFO(logger) <<"test end";
  return 1;
}
