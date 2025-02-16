#include"head.hpp"

static gaiya::Logger::ptr logger = LOG_M()->getLogger("master");

void testTcpServer(){
  LOG_INFO(logger)<<"start";
  gaiya::UnixAddress::ptr unixaddr(new gaiya::UnixAddress("/tmp/test_unixaddr"));
  auto addr = gaiya::Address::LookupAny("0.0.0.0:8030");
  std::vector<gaiya::Address::ptr> socks;
  std::vector<gaiya::Address::ptr> fails;
  socks.push_back(addr);
  socks.push_back(unixaddr);
  gaiya::TcpServer::ptr tcpserver(new gaiya::TcpServer());

  while(!tcpserver->bind(socks,fails)){
    sleep(2);
  }

  tcpserver->start();
  LOG_INFO(logger) <<"error: " <<tcpserver->getSocks()[0]->getError();
  LOG_INFO(logger)<<"============";
  LOG_INFO(logger)<<"end";
}

int main(int argc, char** argv){
  gaiya::IOmanager iom(1);
  iom.schedule(testTcpServer);
  return 1;
}
