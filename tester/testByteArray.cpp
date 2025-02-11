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


void stringTest(){
  gaiya::ByteArray::ptr barray(new gaiya::ByteArray(4));
  std::string send;
  send = "nihaohhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhh";
  std::filesystem::path cur = std::filesystem::path(__FILE__).parent_path();
  cur.append("te.txt");
  lOG_INFO_ROOT()<<cur;
  lOG_INFO_ROOT()<<send;
  lOG_INFO_ROOT()<<barray->getSize();
  barray->writeStringWithnotLength(send);
  barray->setPosition(0);
  if(!barray->writeToFile(cur.string())){
    return;
  }
  barray->clear();
  lOG_INFO_ROOT()<<barray->getSize();
  barray->readFromFile(cur.string());
  barray->setPosition(0);
  std::string recv= barray->readStringWithnotLength();
  lOG_INFO_ROOT()<<barray->getSize();
  lOG_INFO_ROOT()<<recv;

}

void testSocket(){
  lOG_INFO_ROOT() <<"testSocket start";

  gaiya::Socket::ptr sock = gaiya::Socket::CreateTCPScoket();
  std::multimap<std::string,std::pair<gaiya::Address::ptr,uint32_t>> multi;
  gaiya::Address::GetInterfaceAddress(multi,AF_INET);
  sock->bind(multi.begin()->second.first);
  gaiya::Address::ptr baidu = gaiya::Address::LookupAny("www.baidu.com",sock->getFamily(),sock->getType(),sock->getProtocol(),"http");
  lOG_INFO_ROOT() <<baidu->toString();
  if(sock->connect(baidu)){
    lOG_INFO_ROOT() <<"connected...";
  }else{
    lOG_INFO_ROOT() <<"connected failed";
  }
  std::string str = "GET / HTTP/1.0\r\n\r\n";
  sock->send(str.c_str(),str.length());
  std::string recv;
  recv.resize(4096);
  sock->recv(recv.data(),recv.size());
  lOG_INFO_ROOT()<<"recv message:" <<recv;

  lOG_INFO_ROOT() <<"testSocket end";
}


int main(){
  gaiya::setThreadHook(true);
  gaiya::IOmanager::ptr iom(new gaiya::IOmanager(1));
  gaiya::Config::loadYamlFile("/home/luo/cplus/gaiya/src/configuration.yaml");
  iom->schedule(testSocket);
  return 1;
}
