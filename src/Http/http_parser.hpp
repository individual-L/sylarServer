#ifndef __HTTP_PARSER_H__
#define __HTTP_PARSER_H__

//http协议解析封装

#include"http11_parser.h"
#include"httpclient_parser.h"
#include"http.hpp"
#include<stdint.h>

namespace gaiya{
namespace http{
  
class HttpRequestParser{
  public:
    typedef std::shared_ptr<HttpRequestParser> ptr;
    HttpRequestParser();
    int hasError(); 
    int isFinished();
    size_t execute(char* data, size_t len);
    HttpRequest::ptr getData() const { return m_request;}
    const http_parser& getParser() const { return m_parser;}
    uint64_t getContentLength();

    void setError(int v) { m_error = v;}

    static uint64_t GetHttpRequestBufferSize();

    static uint64_t GetHttpRequestBodyMaxSize();
  private:
  http_parser m_parser;
  HttpRequest::ptr m_request;
  /*
  1 invaild method
  2 invaild version
  3 invaild field
  */
  uint32_t m_error;

};

class HttpResponseParser{
  public:
    typedef std::shared_ptr<HttpResponseParser> ptr;
    HttpResponseParser();
    int hasError(); 
    int isFinished();
    size_t execute(char* data, size_t len,bool chunk);
    HttpResponse::ptr getData() const { return m_response;}
    const httpclient_parser& getParser() const { return m_parser;}
    uint64_t getContentLength();

    void setError(int v) { m_error = v;}  

    static uint64_t GetHttpResponseBufferSize();

    static uint64_t GetHttpResponseBodyMaxSize();
  private:
  httpclient_parser m_parser;
  HttpResponse::ptr m_response;
  int m_error;

};

}
}



#endif
