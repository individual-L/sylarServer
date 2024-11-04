#include"head.hpp"

gaiya::Logger::ptr logger = LOG_GET_LOGGER("master");

void fun1(){

    LOG_INFO(logger) <<"fun1 start";
    LOG_INFO(logger) <<"==============================";
    gaiya::Coroutine::GetCur()->YieldToReady();
    LOG_INFO(logger) <<"==============================";
    gaiya::Coroutine::GetCur()->YieldToHold();
    LOG_INFO(logger) <<"==============================";
    LOG_INFO(logger) <<"fun1 end";

}


int main(){
  gaiya::Config::loadYamlFile("/home/luo/cplus/gaiya/tester/configTest.yaml");
  gaiya::Coroutine::GetCur();
  LOG_INFO(logger) <<"coroutine test start";
  std::function<void()> func(fun1);
  gaiya::Coroutine::ptr rt(new gaiya::Coroutine(func));

  LOG_INFO(logger) <<"swapIn1";
  rt->swapIn();

  LOG_INFO(logger) <<"swapIn2";
  rt->swapIn();


  LOG_INFO(logger) <<"swapIn3";
  rt->swapIn();

  LOG_INFO(logger) << gaiya::Coroutine::GetCurId() <<" use_count: "<< gaiya::Coroutine::GetCur().use_count() ;
  LOG_INFO(logger) <<"coroutine test end";
  return 1;
}
