#include"http_server.hpp"
#include"http_session.hpp"
#include"http_serv_function.hpp"

namespace gaiya{
namespace http{

static gaiya::Logger::ptr logger = LOG_M()->getLogger("master");

HttpServer::HttpServer(bool keepalive,gaiya::IOmanager* worker)
:TcpServer(worker)
,m_isKeepalive(keepalive) {
  m_servDispatch.reset(new ServDispatch);

}

void HttpServer::handleClient(Socket::ptr client){
  gaiya::http::HttpSession::ptr session(new gaiya::http::HttpSession(client,true));
  while(true){
    HttpRequest::ptr request = session->recvRequest();
    if(!request) {
        LOG_ERROR(logger) << "recv http request fail, errno="
            << errno << " errstr=" << strerror(errno)
            << " cliet:" << client << " keep_alive=" << m_isKeepalive;
        break;
    }
    HttpResponse::ptr response(new HttpResponse(request->getVersion(),request->isClose() || !m_isKeepalive));
    gaiya::s_servDispatch::getInstance()->handle(request,response);
    // LOG_INFO(logger) <<response->getBody();
    session->sendResponse(response);
  }


}



}
  

}
