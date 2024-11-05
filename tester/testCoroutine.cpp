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
void fun2(int id){
  {
    //获取当前正在执行的协程
  gaiya::Coroutine::GetCur();

  gaiya::Coroutine::ptr rt(new gaiya::Coroutine(&fun1));
  LOG_INFO(logger) <<"id: "  << id <<" swapIn 1";
  rt->swapIn();

  LOG_INFO(logger) <<"id: " << id <<" swapIn 2";
  rt->swapIn();


  LOG_INFO(logger) <<"id: " << id <<" swapIn 3";
  rt->swapIn();

  LOG_INFO(logger)<<"id: " << id <<" " << gaiya::Coroutine::GetCurId() <<" use_count: "<< gaiya::Coroutine::GetCur().use_count() ;
  }
}

int main(){
  gaiya::Thread::SetName("main");
  gaiya::Config::loadYamlFile("/home/luo/cplus/gaiya/tester/configTest.yaml");
  std::vector<gaiya::Thread::ptr> tv;
  for(int i = 0;i < 5;++i){
    tv.push_back(gaiya::Thread::ptr(new gaiya::Thread(std::bind(fun2,i + 1),"thread_" + std::to_string(i + 1))));
  }
  for(size_t i = 0;i < tv.size();++i){
    tv[i]->join();
  }
  return 1;
}
