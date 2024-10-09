#include<iostream>
#include"log.h"
#include"util.h"
int main(int argc, char** argv){
  std::cout << "start test log" << std::endl;
  LOG_DUBUG(LOG_ROOT())<<"hello world!";
  LOG_ERROR(LOG_ROOT())<<"hello error world!";
  LOG_FMT_DUBUG(LOG_ROOT(),"%s","nihao");
  std::cout << "end test log" << std::endl;
  return 1;
}
