#include<iostream>
#include<stdint.h>
#include<string>
#include<sstream>

int main(){
  int32_t mem = -3;
  int32_t cmem = -3 >> 1;
  std::stringstream ss;
  for(int i = 0;i < 8;++i){
    if(mem & (1<< 8 - (i + 1))){
      ss << "1";
    }else{
      ss <<"0";
    }
  }
  std::cout<<"mem: " <<(mem)<<std::endl;

  std::cout<<"mem dec: " <<ss.str()<<std::endl;

  ss.str("");
  for(int i = 0;i < 8;++i){
    if(cmem & (1<< 8 - (i + 1))){
      ss << "1";
    }else{
      ss <<"0";
    }
  }
  std::cout<<"cmem: " <<(cmem)<<std::endl;

  std::cout<<"cmem dec: " <<ss.str()<<std::endl;

  return 1;
}
