#include"head.hpp"

static gaiya::Logger::ptr logger = LOG_GET_LOGGER("master");
void testHook(){
  LOG_INFO(logger) << "testHook start";
  LOG_INFO(logger) << "testHook end";
}

int main(){
  testHook();
  return 1;
}
