#include"head.hpp"

static gaiya::Logger::ptr logger = LOG_M()->getLogger("master");

void testHttpServer(){
  LOG_INFO(logger)<<"start";
  // gaiya::Address::ptr addr = gaiya::Address::GetInterfaceAnyAddress();
  auto addr = gaiya::Address::LookupAny("192.168.80.135:8030");
  std::vector<gaiya::Address::ptr> socks;
  std::vector<gaiya::Address::ptr> fails;
  socks.push_back(addr);
  gaiya::TcpServer::ptr server(new gaiya::http::HttpServer(true));

  while(!server->bind(socks,fails)){
    sleep(2);
  }

  server->start();
  LOG_INFO(logger)<<"end";
}


int main(int argc,char** argv){
  gaiya::LoadConfigrationFile();
  gaiya::IOmanager iom(2);
  iom.schedule(&testHttpServer); 
}
