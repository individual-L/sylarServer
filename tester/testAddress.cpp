#include"head.hpp"

static gaiya::Logger::ptr logger = gaiya::s_LoggersM::getInstance()->getLogger("master");

int main(){
  // std::vector<gaiya::Address::ptr> addrs;
  gaiya::IPAddress::ptr addr =  gaiya::Address::LookupAnyIPAddress("www.pku.edu.cn",AF_INET6,SOCK_STREAM,IPPROTO_TCP);
  if(!addr){
    LOG_INFO(logger) <<"lookup failed";
  }
  // for(auto & it : addr){
    LOG_INFO(logger) <<"ipAddress: " <<addr->toString() <<std::endl
                    <<"netMask: " <<addr->getnetMask(64)->toString() <<std::endl
                    <<"BroadcastAddress: " <<addr->getBroadcastAddress(64)->toString() <<std::endl
                    <<"networkAddress: " <<addr->getnetworkAddress(64)->toString() <<std::endl;
  // }
}
