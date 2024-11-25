#include"timer.hpp"

namespace gaiya{

Timer::Timer(uint64_t period,std::function<void()> cb,bool recur,TimerMng* Mng)
:m_period(period)
,m_recur(recur)
,m_timerMng(Mng){
  m_cb.swap(cb);
  Timer(gaiya::GetCurrentTime() + period);
}

Timer::Timer(uint64_t goTime){
  m_goTime = goTime;
}

Timer::~Timer(){
  m_timerMng = nullptr;
}

bool Timer::Compare::operator()(const Timer::ptr& lptr,const Timer::ptr& rptr) const {
  if(!lptr && !rptr){
    return false;
  }
  if(!rptr){
    return false;
  }
  if(!lptr){
    return true;
  }

  if(lptr->m_goTime < rptr->m_goTime){
    return true;
  }
  if(lptr->m_goTime > rptr->m_goTime){
    return false;
  }
  return lptr.get() < rptr.get();

}


//从定时队列中删除定时器
bool Timer::cancelTimer(){
  TimerMng::MutexType::WriteLock lock(m_timerMng->m_mutex);

  auto it = m_timerMng->m_timers.find(shared_from_this());

  if(it != m_timerMng->m_timers.end()){
    m_timerMng->m_timers.erase(it);
    return true;
  }

  return false;
}

bool Timer::reset(uint64_t period,bool from_now){
  if(period == m_period && !from_now){
    return true;
  }
  TimerMng::MutexType::WriteLock lock(m_timerMng->m_mutex);

  if(!m_cb){
    return false;
  }

  auto it = m_timerMng->m_timers.find(shared_from_this());

  if(it == m_timerMng->m_timers.end()){
    return false;
  }
  m_timerMng->m_timers.erase(it);

  uint64_t startTime = 0;
  if(from_now){
    startTime = gaiya::GetCurrentTime();
  }else{
    startTime = m_goTime - m_period;
  }
  m_period = period;
  m_goTime = startTime + m_period;
  m_timerMng->addTimer(shared_from_this(),lock);
  // time_t ti(m_goTime / 1000ull);
  // gaiya::LogEvent::ptr loginfo(new gaiya::LogEvent(LOG_ROOT(),gaiya::LogLevel::INFO,GET_RELATEIVE,__LINE__,0,"--",0,ti));
  // gaiya::LogEventWrap(loginfo).getSS() <<"==============================";
  return true;
}

bool Timer::refresh(){
  TimerMng::MutexType::WriteLock lock(m_timerMng->m_mutex);
  if(!m_cb){
    return false;
  }

  auto it = m_timerMng->m_timers.find(shared_from_this());

  if(it == m_timerMng->m_timers.end()){
    return false;
  }
  m_timerMng->m_timers.erase(it);

  m_goTime = gaiya::GetCurrentTime() + m_period;
  m_timerMng->addTimer(shared_from_this(),lock);
  return true;
}

TimerMng::TimerMng(){
  m_previousTime = gaiya::GetCurrentTime();
}
TimerMng::~TimerMng(){

}

Timer::ptr TimerMng::addTimer(uint64_t period,std::function<void()> cb,bool recur){
  Timer::ptr timer(new gaiya::Timer(period,cb,recur,this));
  MutexType::WriteLock lock(m_mutex);
  addTimer(timer,lock);
  return timer;

}
static void CondFunc(std::function<void()> cb,std::weak_ptr<void> weak_cond){
  //weak_cond指向的资源是否还存在
  std::shared_ptr<void> it = weak_cond.lock();
  if(it){
    cb();
  }
}

Timer::ptr TimerMng::addConditionTimer(uint64_t period,std::function<void()> cb
  ,std::weak_ptr<void> weak_cond,bool recur){
    return addTimer(period,std::bind(CondFunc,cb,weak_cond),recur);    
}


uint64_t TimerMng::getNextTime(){
  MutexType::ReadLock lock(m_mutex);
  m_tickled = false;
  if(m_timers.empty()){
    //unsigned long long;它一定是64位的
    return ~0ull;
  }
  const auto first = m_timers.begin();
  if((*first)->m_goTime <= gaiya::GetCurrentTime()){
    return 0;
  }
  return (*first)->m_goTime - gaiya::GetCurrentTime();
}
                      

void TimerMng::getTriggerableCB(std::vector<std::function<void()>>& cbs){
  {
    MutexType::ReadLock lock(m_mutex);
    if(m_timers.empty()){
      return;
    }
  }
  MutexType::WriteLock lock(m_mutex);
  if(m_timers.empty()){
    return;
  }
  uint64_t nowTime = gaiya::GetCurrentTime();
  auto begin = m_timers.begin();
  if(!detectClockRollover(nowTime) && (*begin)->m_goTime > nowTime){
    return;
  }

  std::vector<Timer::ptr> timers;
  Timer::ptr now_ptr(new Timer(nowTime));
  auto end = m_timers.lower_bound(now_ptr);
  while(end != m_timers.end() && (*end)->m_goTime >= nowTime){
    ++end;
  }

  timers.insert(timers.begin(),begin,end);
  m_timers.erase(begin,end);
  cbs.reserve(timers.size());

  for(auto& it : timers){
    cbs.push_back(it->m_cb);
    if(it->m_recur){
      it->m_goTime = nowTime + it->m_period;
      m_timers.insert(it);
    }else{
      it->m_cb = nullptr;
    }
  }
  
}

void TimerMng::addTimer(Timer::ptr timer,MutexType::WriteLock& lock){
  auto it = m_timers.insert(timer).first;
  bool at_front = (it == m_timers.begin()) && !m_tickled;

  if(at_front){
    m_tickled = true;
  }
  lock.unlock();

  if(m_tickled){
    onTimersInsertedAtFront();
  }
}
bool TimerMng::detectClockRollover(uint64_t nowTime){
  bool isRoll = false;
  if(nowTime < m_previousTime){
    isRoll = true;
  }
  m_previousTime = nowTime;
  return isRoll;
}

bool TimerMng::hasTimer(){
  MutexType::ReadLock lock(m_mutex);
  return !m_timers.empty();
}



}
