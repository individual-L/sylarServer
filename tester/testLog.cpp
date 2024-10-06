#include<iostream>
#include"log.h"
#include"util.h"
int main(int argc, char** argv){
  gaiya::LoggerManager lmr = gaiya::LoggerManager();
  std::cout << "start test log" << std::endl;
  LOG_DUBUG(lmr.getLogger())<<"hello world!";
  LOG_ERROR(lmr.getLogger())<<"hello error world!";
  LOG_FMT_DUBUG(lmr.getLogger(),"%s","nihao");
  std::cout << "end test log" << std::endl;
  return 1;
}
