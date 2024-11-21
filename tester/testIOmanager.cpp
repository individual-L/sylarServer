#include"head.hpp"
#include<sys/socket.h>
#include<arpa/inet.h>
static gaiya::Logger::ptr logger = LOG_GET_LOGGER("master");

int sock = 0;
void func1(){
  LOG_INFO(logger) <<"func1 start";
  sock = socket(AF_INET,SOCK_STREAM,0);

  sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(80);
  inet_pton(AF_INET,"183.2.172.42",&addr.sin_addr.s_addr);
  if(!connect(sock,(const sockaddr*)&addr,sizeof(addr))){
    LOG_INFO(logger) <<sock <<" connected";
  }else if(errno == EINPROGRESS){
    //在建立连接中
    LOG_INFO(logger) <<"errno = EINPROGRESS";
    gaiya::IOmanager::GetThis()->addEvent(sock,gaiya::IOmanager::READ,[](){
      LOG_INFO(logger) <<"READ triggered";
    });
    gaiya::IOmanager::GetThis()->addEvent(sock,gaiya::IOmanager::WRITE,[&](){
      LOG_INFO(logger) <<"WRITE triggered";
      gaiya::IOmanager::GetThis()->cancelEvent(sock,gaiya::IOmanager::READ);
      close(sock);
    });
  }else{
    LOG_INFO(logger) <<"else: " <<errno <<"(" <<strerror(errno) <<")";
  }
  LOG_INFO(logger) <<"func1 end";
}

void testFunc(){
  LOG_INFO(logger) <<"test start";
  gaiya::IOmanager::ptr iomanager(new gaiya::IOmanager(3,false,"test"));
  iomanager->schedule(func1);
  iomanager->stop();
  LOG_INFO(logger) <<"test end";
}
int main(){
  gaiya::Config::loadYamlFile("/home/luo/cplus/gaiya/tester/configTest.yaml");
  gaiya::Thread::SetName("main");
  testFunc();
  return 1;
}
