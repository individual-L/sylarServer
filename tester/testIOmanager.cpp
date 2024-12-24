#include"head.hpp"
#include<sys/socket.h>
#include<arpa/inet.h>
static gaiya::Logger::ptr logger = LOG_GET_LOGGER("master");

gaiya::IOmanager::ptr iom;
void func1(){
  LOG_INFO(logger) <<"fun1 start";
  int sock = socket(AF_INET,SOCK_STREAM,0);

  sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(80);
  inet_pton(AF_INET,"183.2.172.185",&addr.sin_addr.s_addr);
  int res = connect(sock,(const sockaddr*)&addr,sizeof(addr));
  if(res){
    LOG_INFO(logger) <<"connect errno: "<<errno <<" translate-> " <<strerror(errno);
    return;
  }
  const char msg[] = "GET / HTTP/1.0\r\n\r\n";
  res = send(sock,msg,sizeof(msg),0);

  if(res < 0){
    LOG_INFO(logger) <<"send errno: "<<errno <<" translate-> " <<strerror(errno);
    return;
  }
  std::string buf;
  buf.resize(4096);
  res = recv(sock,&buf[0],buf.size(),0);


  if(res <= 0){
    LOG_INFO(logger) <<"recv errno: "<<errno<<" translate-> " <<strerror(errno);
    return;
  }

  buf.resize(res);
  LOG_INFO(logger) <<"buf: "<<buf;
  LOG_INFO(logger) <<"fun1 end";
  return;

}

gaiya::Timer::ptr tptr = nullptr;

void testFunc(){
  LOG_INFO(logger) <<"test start";
  gaiya::IOmanager::ptr iomanager(new gaiya::IOmanager(2,false,"test"));
  tptr = iomanager->addTimer(3000,[&](){
    static int i = 3;
    // LOG_INFO(logger) <<"timer triggered ---------------- i: " <<i ;
    if((i--) > 0){
      tptr->reset(2000,true);
    }else{
      tptr->reset(2000,true);
    }
  },true);
  LOG_INFO(logger) <<"test end";
}

int main(){
  // gaiya::Config::loadYamlFile("/home/luo/cplus/gaiya/src/configuration.yaml");
  // LOG_INFO(logger) <<"timeout: " <<gaiya::Config::checkBase("tcp.connect.timeout_ms")->toString();
  // gaiya::Thread::SetName("main");
  // iom = gaiya::IOmanager::ptr(new gaiya::IOmanager(1));
  // iom->schedule(func1);
  // int sock = socket(AF_INET,SOCK_STREAM,0);

  sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(80);
  inet_pton(AF_INET,"183.2.172.185",&addr.sin_addr.s_addr);
  uint32_t laddr = gaiya::byteSwapHN(addr.sin_addr.s_addr);
  LOG_INFO(logger) << (laddr & 0xff) <<"."
                  << ((laddr >> 8) & 0xff) <<"."
                  << ((laddr >> 16)& 0xff) <<"."
                  << ((laddr >> 24)& 0xff) <<"."
                  <<":" <<gaiya::byteSwapHN(addr.sin_port);
  return 1;
}
