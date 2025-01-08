#include"head.hpp"

void test(){
  #define XX(type,readFun,writeFun,baseLen) \
  std::vector<type> vec; \
  srand(static_cast<uint64_t>(time(NULL))); \
  for(size_t i = 0;i < 5;++i){ \
    vec.push_back(rand()); \
    lOG_INFO_ROOT() <<vec[i]; \
  } \
  gaiya::ByteArray::ptr barray(new gaiya::ByteArray(baseLen)); \
  for(auto& it : vec){ \
    barray->writeFun(it); \
  } \
  lOG_INFO_ROOT()<<"m_size: " <<barray->getSize(); \
  barray->setPosition(0); \
  for(size_t i = 0;i < 5;++i){ \
    type s = barray->readFun(); \
    if(s == vec[i]){ \
      lOG_INFO_ROOT()<< s <<" == " << vec[i]; \
    } \
  } 
  XX(uint64_t,readDouble,writeDouble,4);
}

int main(){
  test();
  return 1;
}
