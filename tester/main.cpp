#include"head.hpp"

void test_request_parser(){
  char buff[] = "GET /index.html HTTP/1.1\r\n"
"Host: www.example.com\r\n"
"User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, ""like Gecko) Chrome/123.0.0.0 Safari/537.36\r\n"
"Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8\r\n"
"Accept-Language: en-US,en;q=0.5\r\n"
"Accept-Encoding: gzip, deflate, br\r\n"
"Connection: keep-alive";
  gaiya::http::HttpRequestParser::ptr rparser(new gaiya::http::HttpRequestParser());
  size_t size = rparser->execute(buff,strlen(buff));
  lOG_INFO_ROOT()<<"size: " <<size;
  lOG_INFO_ROOT()<<"error: " <<rparser->hasError();
  lOG_INFO_ROOT()<<"finished: " <<rparser->isFinished();
  lOG_INFO_ROOT()<<"content-length: "<<rparser->getContentLength(); 
  lOG_INFO_ROOT()<<rparser->getData()->toString();
}



void test_response_parser(){
  char buff[] = "HTTP/1.1 200 OK\r\n"
"Content-Type: text/html; charset=UTF-8\r\n"
"Content-Length: 1234\r\n"
"Server: Apache/2.4.41 (Ubuntu)\r\n"
"Date: Tue, 11 Feb 2025 08:00:00 GMT\r\n"
"Connection: keep-alive\r\n\r\n"
"<!DOCTYPE html>"
"<html lang=\"en\">"
"<head>"
"    <meta charset=\"UTF-8\">"
"    <title>Example Page</title>"
"</head>"
"<body>"
"    <h1>Welcome to Example Page</h1>"
"    <p>This is a sample HTML response.</p>"
"</body>"
"</html>";
  lOG_INFO_ROOT()<<buff <<strlen(buff);
  std::string str = buff;
  gaiya::http::HttpResponseParser::ptr rparser(new gaiya::http::HttpResponseParser());
  size_t size = rparser->execute(str.data(),str.length(),false);
  lOG_INFO_ROOT()<<"parse_size: " <<size;
  lOG_INFO_ROOT()<<"buff[size]: " <<buff[size];
  lOG_INFO_ROOT()<<"error: " <<rparser->hasError();
  lOG_INFO_ROOT()<<"finished: " <<rparser->isFinished();
  lOG_INFO_ROOT()<<"content-length: "<<rparser->getContentLength(); 
  lOG_INFO_ROOT()<<rparser->getData()->toString();
  lOG_INFO_ROOT()<<buff;
}

int main(){
  // test_request_parser();
  // test_response_parser();
  const char* url = "12345: ?#[]1234";
  lOG_INFO_ROOT()<<gaiya::StringUrl::UrlEncode(url,false);
  std::string str = gaiya::StringUrl::UrlEncode(url,false);
  lOG_INFO_ROOT()<<gaiya::StringUrl::UrlDecode(str,false);
  return 1;
}
