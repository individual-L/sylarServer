#include "head.hpp"


static gaiya::Logger::ptr g_looger = LOG_M()->getLogger("master");

void test_socket() {
    //std::vector<gaiya::Address::ptr> addrs;
    //gaiya::Address::Lookup(addrs, "www.baidu.com", AF_INET);
    //gaiya::IPAddress::ptr addr;
    //for(auto& i : addrs) {
    //    LOG_INFO(g_looger) << i->toString();
    //    addr = std::dynamic_pointer_cast<gaiya::IPAddress>(i);
    //    if(addr) {
    //        break;
    //    }
    //}
    gaiya::IPAddress::ptr addr = gaiya::Address::LookupAnyIPAddress("www.baidu.com");
    if(addr) {
        LOG_INFO(g_looger) << "get address: " << addr->toString();
    } else {
        LOG_ERROR(g_looger) << "get address fail";
        return;
    }

    gaiya::Socket::ptr sock = gaiya::Socket::CreateTCP(addr);
    addr->setPort(80);
    LOG_INFO(g_looger) << "addr=" << addr->toString();
    if(!sock->connect(addr)) {
        LOG_ERROR(g_looger) << "connect " << addr->toString() << " fail";
        return;
    } else {
        LOG_INFO(g_looger) << "connect " << addr->toString() << " connected";
    }

    const char buff[] = "GET / HTTP/1.0\r\n\r\n";
    int rt = sock->send(buff, sizeof(buff));
    if(rt <= 0) {
        LOG_INFO(g_looger) << "send fail rt=" << rt;
        return;
    }

    std::string buffs;
    buffs.resize(4096);
    rt = sock->recv(&buffs[0], buffs.size());

    if(rt <= 0) {
        LOG_INFO(g_looger) << "recv fail rt=" << rt;
        return;
    }

    buffs.resize(rt);
    LOG_INFO(g_looger) << buffs;
}


int main(int argc, char** argv) {
    gaiya::IOmanager iom;
    //iom.schedule(&test_socket);
    iom.schedule(&test_socket);
    return 0;
}
