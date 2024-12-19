#include"head.hpp"

static gaiya::Logger::ptr logger = LOG_GET_LOGGER("master");

void testHook(){
  gaiya::IOmanager::ptr iom(new gaiya::IOmanager());
  LOG_INFO(logger) << "testHook start";
  iom->schedule([](){
    sleep(2);
    LOG_INFO(logger) << "2 sleep";
  });
  iom->schedule([](){
    sleep(3);
    LOG_INFO(logger) << "3 sleep";
  });
  LOG_INFO(logger) << "last sleep";
  LOG_INFO(logger) << "testHook end";
}

int main(){
  gaiya::setThreadHook(true);
  // testHook();
  return 1;
}
