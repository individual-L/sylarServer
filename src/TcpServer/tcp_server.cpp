#include"tcp_server.hpp"
#include"config.hpp"
#include"log.hpp"
#include"fdMng.hpp"
namespace gaiya{

static gaiya::Logger::ptr logger = LOG_M()->getLogger("master");

static gaiya::ConfigVar<uint64_t>::ptr g_tcp_server_recv_timeout =
    gaiya::Config::lookup("tcp_server.recv_timeout","tcp server read timeout", (uint64_t)(60 * 1000 * 2));

TcpServer::TcpServer(IOmanager* iom,IOmanager* accept_iom)
:m_worker(iom)
,m_accept(accept_iom)
,m_name("gaiya/1.0.0")
,m_isStop(true)
,m_recvTimeout(g_tcp_server_recv_timeout->getValue()){


}

TcpServer::~TcpServer() {
    for(auto& i : m_socks) {
        i->close();
    }
    m_socks.clear();
}

bool TcpServer::bind(Address::ptr addr){
  std::vector<Address::ptr> socks;
  std::vector<Address::ptr> fails;
  socks.push_back(addr);
  return bind(socks,fails);
}
bool TcpServer::bind(const std::vector<Address::ptr>& addrs,std::vector<Address::ptr>& fails){
  for(auto& it : addrs){
    Socket::ptr sock = Socket::CreateTCP(it);
    if(!sock->bind(it)){
      LOG_ERROR(logger) << "bind fail errno="
                          << errno << " errstr=" << strerror(errno)
                          << " addr=[" << it->toString() << "]";
      fails.push_back(it);
      continue;
    }
    sock->setRecvTimeout(-1);
    if(!sock->listen()){
      LOG_ERROR(logger) << "listen fail errno="
                          << errno << " errstr=" << strerror(errno)
                          << " addr=[" << it->toString() << "]";
      fails.push_back(it);
      continue;
    }
    m_socks.push_back(sock);
  }
  //是否有地址bind失败
  if(!fails.empty()) {
    m_socks.clear();
    return false;
  }
  for(auto& i : m_socks) {
    LOG_INFO(logger)
        << " ServerName=" << m_name
        << " server bind success: " <<i;
  }
  return true;

}
void TcpServer::startAccept(Socket::ptr sock){
  while(!m_isStop){
    // LOG_INFO(logger)<<sock->getSockfd()<<" start accept...";
    Socket::ptr client = sock->accept();
    if(client) {
        client->setRecvTimeout(m_recvTimeout);
        m_worker->schedule(std::bind(&TcpServer::handleClient,shared_from_this(),client));
    } else {
        LOG_ERROR(logger) << "accept errno=" << errno
            << " errstr=" << strerror(errno);
    }
  }

}
bool TcpServer::start() {
    if(!m_isStop) {
        return true;
    }
    m_isStop = false;
    // LOG_INFO(logger) <<m_socks.size();
    for(auto & it : m_socks){
      m_worker->schedule(std::bind(&TcpServer::startAccept,shared_from_this(),it));
    }
    return true;
}
void TcpServer::stop(){
  if(m_isStop){
    return;
  }
  //在close掉监听fd之前，保证tcpserver不会被销毁
  auto self = shared_from_this();
  m_accept->schedule([self,this](){
    for(auto& it : m_socks){
      it->cancelAll();
      it->close();
    }
    m_socks.clear();
  });
  m_isStop = true;
}
void TcpServer::handleClient(Socket::ptr client) {
    LOG_INFO(logger) << "handleClient: " <<client;
}




}
