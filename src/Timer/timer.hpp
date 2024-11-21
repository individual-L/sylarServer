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

    bool operator < (const Timer& right);

  private:
    bool refresh();

    Timer(uint64_t period,std::function<void()> cb,bool recur,TimerMng* Mng);
    Timer(uint64_t goTime);
    Timer();
    ~Timer();

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
    void getTriggerableCB(std::vector<std::function<void()>>& timers);

    bool hasTimer();
  private:
    bool addTimer(Timer::ptr timer,MutexType::WriteLock lock);

  private:
    MutexType m_mutex;
    std::set<Timer::ptr> m_timers;
};

}


#endif
