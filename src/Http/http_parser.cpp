#include"http_parser.h"
#include"log.hpp"
#include"http.hpp"
#include"config.hpp"

namespace gaiya{
namespace http{
  
static gaiya::Logger::ptr logger = LOG_M()->getLogger("master");

static gaiya::ConfigVar<uint64_t>::ptr r_http_request_buffer_size = gaiya::Config::lookup(
  "http.request.buffer_size","http request buffer size",(uint64_t)(4 * 1024));
static gaiya::ConfigVar<uint64_t>::ptr r_http_request_body_maxSize = gaiya::Config::lookup(
  "http.request.body_maxsize","http request body maxSize",(uint64_t)(64 * 1024*1024));
static gaiya::ConfigVar<uint64_t>::ptr r_http_response_buffer_size = gaiya::Config::lookup(
  "http.response.buffer_size","http response buffer size",(uint64_t)(4 * 1024)); 
static gaiya::ConfigVar<uint64_t>::ptr r_http_response_body_maxSize = gaiya::Config::lookup(
  "http.response.body_maxsize","http response body maxSize",(uint64_t)(64 * 1024*1024));

static uint64_t s_http_request_buffer_size = 0;
static uint64_t s_http_request_body_maxSize = 0;
static uint64_t s_http_response_buffer_size = 0;
static uint64_t s_http_response_body_maxSize = 0;


//匿名空间防止文件外的文件访问到此空间的_init变量
namespace {
struct RequestSizeIniter {
    RequestSizeIniter() {
        s_http_request_buffer_size = r_http_request_buffer_size->getValue();
        s_http_request_body_maxSize = r_http_request_body_maxSize->getValue();
        s_http_response_buffer_size = r_http_response_buffer_size->getValue();
        s_http_response_body_maxSize = r_http_response_body_maxSize->getValue();

        r_http_request_buffer_size->addCallBackFunc(
                [](const uint64_t& old, const uint64_t& update){
                s_http_request_buffer_size = update;
        });

        r_http_request_body_maxSize->addCallBackFunc(
                [](const uint64_t& old, const uint64_t& update){
                s_http_request_body_maxSize = update;
        });

        r_http_response_buffer_size->addCallBackFunc(
                [](const uint64_t& old, const uint64_t& update){
                s_http_response_buffer_size = update;
        });

        r_http_response_body_maxSize->addCallBackFunc(
                [](const uint64_t& old, const uint64_t& update){
                s_http_response_body_maxSize = update;
        });
    }
};
static RequestSizeIniter _init;
}

uint64_t HttpRequestParser::GetHttpRequestBufferSize() {
    return s_http_request_buffer_size;
}

uint64_t HttpRequestParser::GetHttpRequestBodyMaxSize() {
    return s_http_request_body_maxSize;
}

uint64_t HttpResponseParser::GetHttpResponseBufferSize() {
    return s_http_response_buffer_size;
}

uint64_t HttpResponseParser::GetHttpResponseBodyMaxSize() {
    return s_http_response_body_maxSize;
}

void on_http_request_field(void *data, const char *field, size_t flen, const char *value, size_t vlen){
  HttpRequestParser * hrp = static_cast<HttpRequestParser *>(data);
  if(flen == 0){
    LOG_WARN(logger) <<"invaild http field"
                    <<field <<":"<<value;
    hrp->setError(3);
    return;
  }
  hrp->getData()->setHeader(std::string(field,flen),std::string(value,vlen));
}

void on_request_method(void *data, const char *at, size_t length){
  std::string str = std::string(at,length);
  HttpRequestParser * hrp = static_cast<HttpRequestParser *>(data);
  HttpMethod method = StringToHttpMethod(std::string(at,length));
  if(method == HttpMethod::INVAILD_METHOD){
    LOG_WARN(logger) << "invalid http request method: "
                    << std::string(at, length);
    hrp->setError(1);
    return; 
  }
  hrp->getData()->setMethod(method);
}

void on_request_uri(void *data, const char *at, size_t length){

}
void on_fragment(void *data, const char *at, size_t length){
  HttpRequestParser * hrp = static_cast<HttpRequestParser *>(data);
  hrp->getData()->setFragment(std::string(at, length));

}
void on_request_path(void *data, const char *at, size_t length){
  HttpRequestParser * hrp = static_cast<HttpRequestParser *>(data);
    hrp->getData()->setPath(std::string(at, length));
}
void on_query_string(void *data, const char *at, size_t length){
  HttpRequestParser * hrp = static_cast<HttpRequestParser *>(data);
  hrp->getData()->setQuery(std::string(at, length));
}
void on_http_request_version(void *data, const char *at, size_t length){
  HttpRequestParser * hrp = static_cast<HttpRequestParser *>(data);
  uint8_t v = 0;
  if(strncmp(at, "HTTP/1.1", length) == 0) {
      v = 0x11;
  } else if(strncmp(at, "HTTP/1.0", length) == 0) {
      v = 0x10;
  } else {
      LOG_WARN(logger) << "invalid http request version: "
          << std::string(at, length);
      hrp->setError(2);
      return;
  }
  hrp->getData()->setVersion(v);
}
void on_header_request_done(void *data, const char *at, size_t length){

}

HttpRequestParser::HttpRequestParser()
:m_error(0){
  m_request.reset(new HttpRequest());
  http_parser_init(&m_parser);
  m_parser.data = this;
  m_parser.request_method = on_request_method;
  m_parser.request_uri = on_request_uri;
  m_parser.fragment = on_fragment;
  m_parser.request_path = on_request_path;
  m_parser.query_string = on_query_string;
  m_parser.http_version = on_http_request_version;
  m_parser.header_done = on_header_request_done;
  m_parser.http_field = on_http_request_field;
}
int HttpRequestParser::hasError(){
  return m_error || http_parser_has_error(&m_parser);
}
int HttpRequestParser::isFinished(){
  return http_parser_is_finished(&m_parser);
}

//-1: 有错误
//>0: 成功，已处理的字节数，且data中的有效数据为len - v;
size_t HttpRequestParser::execute(char* data, size_t len){
  //返回已经解析的字节数
  size_t offset = http_parser_execute(&m_parser, data, len, 0);
  memmove(data, data + offset, (len - offset));
  return offset;
}

uint64_t HttpRequestParser::getContentLength(){
  return m_request->getHeaderAs<uint64_t>("content-length",0);
}


void on_http_response_field(void *data, const char *field, size_t flen, const char *value, size_t vlen){
  HttpResponseParser* hrp = static_cast<HttpResponseParser*>(data);
  if(flen == 0) {
      LOG_WARN(logger) << "invalid http response field length == 0";
      hrp->setError(3);
      return;
  }
  hrp->getData()->setHeader(std::string(field, flen),std::string(value, vlen));
}

void on_reason_phrase(void *data, const char *at, size_t length){
  HttpResponseParser* hrp = static_cast<HttpResponseParser*>(data);
  hrp->getData()->setReason(std::string(at, length));
}
void on_status_code(void *data, const char *at, size_t length){
  HttpResponseParser* hrp = static_cast<HttpResponseParser*>(data);
  HttpStatus status = (HttpStatus)(atoi(at));
  hrp->getData()->setStatus(status);
}
void on_chunk_size(void *data, const char *at, size_t length){

}
void on_http_response_version(void *data, const char *at, size_t length){
  HttpResponseParser* hrp = static_cast<HttpResponseParser*>(data);
  uint8_t v = 0;
  if(strncmp(at, "HTTP/1.1", length) == 0) {
      v = 0x11;
  } else if(strncmp(at, "HTTP/1.0", length) == 0) {
      v = 0x10;
  } else {
      LOG_WARN(logger) << "invalid http response version: "
          << std::string(at, length);
      hrp->setError(1001);
      return;
  }
  hrp->getData()->setVersion(v);
}
void on_header_response_done(void *data, const char *at, size_t length){

}
void on_last_chunk(void *data, const char *at, size_t length){

}


HttpResponseParser::HttpResponseParser()
:m_error(0){
  m_response.reset(new HttpResponse());
  httpclient_parser_init(&m_parser);
  m_parser.reason_phrase = on_reason_phrase;
  m_parser.status_code = on_status_code;
  m_parser.chunk_size = on_chunk_size;
  m_parser.http_version = on_http_response_version;
  m_parser.header_done = on_header_response_done;
  m_parser.last_chunk = on_last_chunk;
  m_parser.http_field = on_http_response_field;
  m_parser.data = this;
}
int HttpResponseParser::hasError(){
  return m_error || httpclient_parser_has_error(&m_parser);

}
int HttpResponseParser::isFinished(){
  return httpclient_parser_is_finished(&m_parser);
}

size_t HttpResponseParser::execute(char* data, size_t len,bool chunk){
  if(chunk){
    httpclient_parser_init(&m_parser);
  }
  size_t offset = httpclient_parser_execute(&m_parser,data,len,0);
  memmove(data,data + offset,(len - offset));
  return offset;
}

uint64_t HttpResponseParser::getContentLength(){
  return m_response->getHeaderAs<uint64_t>("content-length",0);
}

}

}
