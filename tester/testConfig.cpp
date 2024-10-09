#include<iostream>
#include"log.h"
#include"util.h"
#include"config.h"
int main(int argc, char** argv){
  std::cout << "start test config" << std::endl;
  gaiya::ConfigVar<int>::ptr port(new gaiya::ConfigVar<int>("port","system's port",(int)8080));
  gaiya::ConfigVar<float>::ptr fll(new gaiya::ConfigVar<float>("float","system's float",3.14f));
  LOG_DUBUG(LOG_ROOT())<<port->getName();
  LOG_DUBUG(LOG_ROOT())<<port->getValue();
  LOG_DUBUG(LOG_ROOT())<<fll->getName();
  LOG_DUBUG(LOG_ROOT())<<fll->getValue();
  LOG_DUBUG(LOG_ROOT())<<gaiya::TypeName<gaiya::ConfigVar<int>>();
  std::cout << "end test config" << std::endl;
  return 1;
}
