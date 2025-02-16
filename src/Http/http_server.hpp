#ifndef __HTTP_SERVER_H__
#define __HTTP_SERVER_H__

#include"tcp_server.hpp"
#include"socket_stream.hpp"
#include"http_serv_function.hpp"

namespace gaiya {
namespace http {

class HttpServer : public TcpServer {
public:
    /// 智能指针类型
    typedef std::shared_ptr<HttpServer> ptr;

    HttpServer(bool keepalive = false
                ,gaiya::IOmanager* worker = gaiya::IOmanager::GetThis());
protected:
    virtual void handleClient(Socket::ptr client) override;
private:
    /// 是否支持长连接
    bool m_isKeepalive;

    //根据请求初始化响应消息
    ServDispatch::ptr m_servDispatch;
};

}
}

#endif
