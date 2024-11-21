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

bool Timer::operator < (const Timer& right){
  if(m_goTime < right.m_goTime){
    return true;
  }else{
    return false;
  }
}

Timer::ptr TimerMng::addTimer(uint64_t period,std::function<void()> cb,bool recur){}

Timer::ptr TimerMng::addConditionTimer(uint64_t period,std::function<void()> cb
  ,std::weak_ptr<void> weak_cond,bool recur){

}
                            
void TimerMng::getTriggerableCB(std::vector<std::function<void()>>& timers){}

bool TimerMng::addTimer(Timer::ptr timer,MutexType::WriteLock lock){

}


bool TimerMng::hasTimer(){
  MutexType::ReadLock lock(m_mutex);
  return m_timers.size() > 0;
}
}
