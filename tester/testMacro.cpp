#include<cassert>
#include"head.hpp"


void func2(){
  lOG_INFO_ROOT() << "start fun2";
  GAIYA_ASSERT2(1 == 0,"hello");
  lOG_INFO_ROOT() << "end fun2";
}

void func1(){
  lOG_INFO_ROOT() << "start fun1";
  GAIYA_ASSERT(1 == 0);
  func2();
  lOG_INFO_ROOT() << "end fun1";
}
int main(){
  lOG_INFO_ROOT() << "start test";
  func1();
  lOG_INFO_ROOT() << "end test";
  return 1;
}
