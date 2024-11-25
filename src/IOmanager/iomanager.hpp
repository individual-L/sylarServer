#ifndef __IOMANAGER_H__
#define __IOMANAGER_H__
#include"schedule.hpp"
#include"timer.hpp"

namespace gaiya{
//基于Epoll的IO协程调度器
class IOmanager :public Scheduler , public TimerMng{
  public:
    typedef std::shared_ptr<IOmanager> ptr;
    typedef gaiya::RWMutex MutexType;
    enum EventType{
      NONE = 0x0,
      READ = 0X1,
      WRITE = 0X4,
    };
    IOmanager(uint32_t threadSize = 1,bool useSche = true,const std::string& name = "main");

    ~IOmanager();

    bool addEvent(int fd,EventType event,std::function<void()> cb);

    bool delEvent(int fd,EventType event);

    bool cancelEvent(int fd, EventType event);

    bool cancelAll(int fd);   

    static IOmanager* GetThis();
  protected:
    void idle() override;

    void tickle() override;


    bool stopping() override;

    void onTimersInsertedAtFront() override;

    bool stopping(uint64_t& timeout);

    void contextResize(size_t size);


  private:
    struct FdContext{
      typedef Mutex MutexType;

      struct EventContext{

        Scheduler* scheduler = nullptr;

        Coroutine::ptr coro;

        std::function<void()> cb;
      };
      EventContext& getContext(const EventType& event);

      void resetContext(EventContext& ctx);

      void triggerEvent(const EventType& event);

      EventContext read;

      EventContext write;

      int fd = 0;

      EventType events = NONE;

      MutexType mutex;
      
    };
    

  public:
    std::atomic<size_t> m_pendingEventCount = {0};

  private:
    int m_epfd = 0;

    int m_tickleFds[2];

    MutexType m_mutex;

    std::vector<FdContext*> m_fdctxs;

};


}

#endif
