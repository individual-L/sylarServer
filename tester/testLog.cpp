#include<iostream>
#include"log.h"
int main(int argc, char** argv){
  gaiya::Logger::ptr logger(new gaiya::Logger());
  gaiya::FileLogAppender::ptr file_appender(new gaiya::FileLogAppender("../tester/log.txt"));
  gaiya::StdLogAppender::ptr std_appender(new gaiya::StdLogAppender);
  gaiya::LogFormater::ptr fmt(new gaiya::LogFormater("%d%T%p%T%m%T%l%n"));
  file_appender->setLogformater(fmt);
  file_appender->setLevel(gaiya::LogLevel::Level::DEBUG);
  logger->addAppenders(file_appender);
  logger->addAppenders(std_appender);

  gaiya::LogEvent::ptr event(new gaiya::LogEvent(logger,gaiya::LogLevel::Level::DEBUG,__FILE__,__LINE__,0,1,time(0)));
  event->getSS()<<" hello log";
  std::cout << "start test log" << std::endl;
  logger->log(gaiya::LogLevel::DEBUG,event);
  logger->log(gaiya::LogLevel::INFO,event);
  std::cout << "end test log" << std::endl;
  return 1;
}
