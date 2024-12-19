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
  LOG_INFO(logger) <<"connect errno: "<<errno <<" translate-> " <<strerror(errno);
  if(res){
    return;
  }
  const char msg[] = "GET / HTTP/1.0\r\n\r\n";
  res = send(sock,msg,sizeof(msg),0);
  LOG_INFO(logger) <<"send errno: "<<errno <<" translate-> " <<strerror(errno);

  std::string buf;
  buf.resize(4096);
  size_t len = sizeof(buf);
  res = recv(sock,&buf[0],len,0);

  LOG_INFO(logger) <<"recv errno: "<<errno<<" translate-> " <<strerror(errno);

  if(res <= 0){
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
  gaiya::Config::loadYamlFile("/home/luo/cplus/gaiya/src/configuration.yaml");
  LOG_INFO(logger) <<"timeout: " <<gaiya::Config::checkBase("tcp.connect.timeout_ms")->toString();
  gaiya::Thread::SetName("main");
  iom = gaiya::IOmanager::ptr(new gaiya::IOmanager(2));
  iom->schedule(func1);
  return 1;
}
