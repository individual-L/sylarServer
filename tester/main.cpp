#include"head.hpp"

void test_http(){
  gaiya::http::HttpRequest::ptr req(new gaiya::http::HttpRequest);
  req->setHeader("host","www.gaiya.com");
  req->setPath("/gaiya/src");
  req->dump(std::cout) <<std::endl;

  gaiya::http::HttpResponse::ptr res(new gaiya::http::HttpResponse);
  res->setBody("hello world");
  res->dump(std::cout) <<std::endl;
}

int main(){
  test_http();

  return 1;
}
