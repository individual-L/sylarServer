#ifndef __FDMNG_H__
#define __FDMNG_H__

#include<memory>
#include<lock.hpp>
#include<vector>
#include"singlem.hpp"

namespace gaiya{

class FdMng;
class FdCtx : public std::enable_shared_from_this<FdCtx> {
  public:
    friend class FdMng;
    typedef std::shared_ptr<FdCtx> ptr;

    FdCtx(int fd);
    FdCtx(){}

    ~FdCtx(){}

    bool isInit() const { return m_isInit;}

    bool isSocket() const { return m_isSocket;}

    bool isClose() const { return m_isClosed;}
  
    bool getSysNonBlock() const {return m_sysNonBlock;}

    void SetSysNonBlock(bool isblock){m_sysNonBlock = isblock;}

    bool getUserNonBlock() const {return m_userNonBlock;}

    void SetUserNonBlock(bool isblock){m_userNonBlock = isblock;}

    void setTimeout(int type, uint64_t v);

    uint64_t getTimeout(int type);
  private:
    bool init();
  private:
    int m_fd = -1;
    bool m_isInit = false;
    bool m_isSocket = false;
    bool m_isClosed = true;
    bool m_sysNonBlock = false;
    bool m_userNonBlock = false;
    uint64_t m_readTimeOut = -1;
    uint64_t m_writeTimeOut = -1;
};
class FdMng{
  public:
    typedef gaiya::RWMutex mutexType; 
    FdMng();

    FdCtx::ptr get(int fd, bool auto_create = false);

    void del(int fd);
  private:
    mutexType m_mutex;

    std::vector<FdCtx::ptr> m_fds;
};

typedef Singlem<FdMng> fdMng;



}

#endif
