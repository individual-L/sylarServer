#include"http_session.hpp"
#include"http_parser.hpp"
#include"log.hpp"

namespace gaiya{
namespace http{

static gaiya::Logger::ptr logger = LOG_M()->getLogger("master");

HttpSession::HttpSession(Socket::ptr sock, bool owner)
:SocketStream(sock,owner){

}

HttpRequest::ptr HttpSession::recvRequest(){
  HttpRequestParser::ptr parser(new HttpRequestParser);
  size_t maxSize = HttpRequestParser::GetHttpRequestBufferSize();
  std::shared_ptr<char> buff_ptr(new char[maxSize],[](char* ptr){
    delete [] ptr;
  });
  size_t offset = 0;
  char* buff = buff_ptr.get();
  while(true){
    int64_t rt = read(buff + offset,maxSize - offset);
    if(rt <= 0) {
      close();
      return nullptr;
    }
    rt += offset;
    size_t parser_len = parser->execute(buff,rt);
    if(parser->hasError()) {
        close();
        return nullptr;
    }
    // LOG_INFO(logger) <<"parser_len: " <<parser_len;
    offset = rt - parser_len;
    if(parser->isFinished()) {
        break;
    }
  }
  
  size_t length = parser->getContentLength();
  if(length > 0) {
    std::string body;
    body.resize(length);

    int len = 0;
    if(length >= offset) {
        memcpy(&body[0], buff, offset);
        len = offset;
    } else {
        memcpy(&body[0], buff, length);
        len = length;
    }
    length -= offset;
    if(length > 0) {
        if(readFixSize(&body[len], length) <= 0) {
            close();
            return nullptr;
        }
    }
    parser->getData()->setBody(body);
  }
  //是否为长连接
  parser->getData()->init();
  // LOG_INFO(logger) <<*parser->getData();
  return parser->getData();
}

int HttpSession::sendResponse(HttpResponse::ptr rsp){
  std::stringstream ss;
  ss << rsp->toString();
  std::string str(ss.str());
  // LOG_INFO(logger) <<str;
  return writeFixSize(str.data(),str.size());
}

}
}
