#include"head.hpp"

static gaiya::Logger::ptr logger = LOG_M()->getLogger("master");


class EachServer : public gaiya::TcpServer{
  public:
    void handleClient(gaiya::Socket::ptr client)override;

};
void EachServer::handleClient(gaiya::Socket::ptr client){
  gaiya::Stream::ptr stream(new gaiya::SocketStream(client,true));
  gaiya::ByteArray::ptr array(new gaiya::ByteArray);
  while(true){
    size_t rt = stream->read(array,1024);
    if(rt == 0){
      LOG_INFO(logger)<<"client data.size: 0";
      break;
    }else if(rt < 0){
      LOG_INFO(logger)<<"client error: " <<client->getError() <<"("
                      <<strerror(client->getError()) <<")";
      break;
    }
    LOG_INFO(logger)<<"rt: " <<rt;
    // LOG_INFO(logger)<<"iovec: " <<(char*)iovecs[0].iov_base;
    // LOG_INFO(logger)<<"str: " <<(char*)buff;
    array->setPosition(0);
    LOG_INFO(logger)<<"toString: " <<array->toString();
    // LOG_INFO(logger)<<"toHexString: " <<array->toHexString();
  }
}

void testTcpServer(){
  LOG_INFO(logger)<<"start";
  gaiya::Address::ptr addr = gaiya::Address::GetInterfaceAnyAddress();
  std::vector<gaiya::Address::ptr> socks;
  std::vector<gaiya::Address::ptr> fails;
  socks.push_back(addr);
  gaiya::TcpServer::ptr tcpserver(new EachServer());

  while(!tcpserver->bind(socks,fails)){
    sleep(2);
  }

  tcpserver->start();
  LOG_INFO(logger)<<"end";
}

int main(){
  gaiya::IOmanager iom(2);
  iom.schedule(&testTcpServer);
}
