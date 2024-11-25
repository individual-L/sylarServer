#ifndef __TIME_H__
#define __TIME_H__

#include<memory>
#include<functional>
#include<set>
#include<lock.hpp>
#include"util.hpp"

namespace gaiya{

class TimerMng;
class Timer :public std::enable_shared_from_this<Timer>{
  friend class TimerMng;
  public:
    typedef std::shared_ptr<Timer> ptr;

    bool cancelTimer();

    bool reset(uint64_t period,bool from_now);
    ~Timer();

  private:
    bool refresh();

    Timer(uint64_t period,std::function<void()> cb,bool recur,TimerMng* Mng);
    Timer(uint64_t goTime);
    Timer();

  private:
    //回调函数
    std::function<void()> m_cb = nullptr;
    //等待时间(毫秒)
    uint64_t m_period = 0;
    //运行时间
    uint64_t m_goTime = 0;
    //是否重复执行
    bool m_recur = false;
    //管理器指针
    TimerMng* m_timerMng = nullptr;

    private:
      struct Compare{
        bool operator()(const Timer::ptr& lptr,const Timer::ptr& rptr) const;
      };
};

class TimerMng{
  friend class Timer;
  public:
    typedef RWMutex MutexType;
    TimerMng();
    ~TimerMng();
    Timer::ptr addTimer(uint64_t period,std::function<void()> cb,bool recur);
    Timer::ptr addConditionTimer(uint64_t period,std::function<void()> cb
                                ,std::weak_ptr<void> weak_cond,bool recur);
    void getTriggerableCB(std::vector<std::function<void()>>& cbs);

    uint64_t getNextTime();
    bool hasTimer();
  private:
    void addTimer(Timer::ptr timer,MutexType::WriteLock& lock);

    bool detectClockRollover(uint64_t now_ms);
  protected:
    virtual void onTimersInsertedAtFront() = 0;

  private:
    MutexType m_mutex;
    std::set<Timer::ptr,Timer::Compare> m_timers;
    bool m_tickled = false;
    uint64_t m_previousTime;
};

}


#endif
