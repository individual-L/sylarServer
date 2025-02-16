#ifndef __TCPSERVER_H__
#define __TCPSERVER_H__

#include<memory>

#include"socket.hpp"
#include"iomanager.hpp"
#include"address.hpp"
#include"util.hpp"

namespace gaiya{

class TcpServer :public std::enable_shared_from_this<TcpServer>,public Noncopyable{
  public: 
    typedef std::shared_ptr<TcpServer> ptr;
    TcpServer(IOmanager* iom = gaiya::IOmanager::GetThis());
    virtual ~TcpServer();
    virtual bool bind(Address::ptr addr);
    virtual bool bind(const std::vector<Address::ptr>& addrs,std::vector<Address::ptr>& fails);
    virtual bool start();
    virtual void stop();
    uint64_t getRecvTimeout() const { return m_recvTimeout;}
    std::string getName() const { return m_name;}

    void setRecvTimeout(uint64_t v) { m_recvTimeout = v;}
    virtual void setName(const std::string& v) { m_name = v;}
    bool isStop() const { return m_isStop;}
    std::vector<Socket::ptr> getSocks() const { return m_socks;}
    void startAccept(Socket::ptr sock);

protected:
    virtual void handleClient(Socket::ptr client);

  private:
    IOmanager* m_worker;
    // IOmanager* m_acceptWorker;
    std::string m_name;
    //监听sockfd
    std::vector<Socket::ptr> m_socks;
    bool m_isStop;
    uint64_t m_recvTimeout;

};

}


#endif
