#ifndef __SCHEDULER_H__
#define __SCHEDULER_H__

#include<string>
#include<vector>
#include<list>


#include"thread.hpp"
#include"lock.hpp"
#include"coroutine.hpp"


namespace gaiya{

class Scheduler{
  public:
  typedef std::shared_ptr<Scheduler> ptr;
  typedef gaiya::Mutex MuteType;
    Scheduler(uint64_t size = 1,const std::string& name = "");
    virtual ~Scheduler();

    const std::string getName() const {return m_name;}

    static Scheduler::ptr GetThis();

    static gaiya::Coroutine::ptr GetMainFunc();

    void start();

    void stop();

    template<class CorF>
    void schedule(CorF val,uint64_t thread){
      //队列中是否有任务需要处理
      bool needToTickle = false;
    {
      MuteType::Lock lock(m_mutex);
      needToTickle = scheduleNonLock(val,thread);
    }
      if(needToTickle){
        tickle();
      }
    }  
    //可以协程智能指针数组迭代器，批量调用协程
    template<class CFIterator>
    void schedule(CFIterator begin,CFIterator end){
      //队列中是否有任务需要处理
      bool needToTickle = false;
      {
        MuteType::Lock lock(m_mutex);
        while(begin != end){
          needToTickle = m_corfs.push_back(CorFType(*begin,-1));
          begin++;
        }
      }
      if(needToTickle){
        tickle();
      }
    } 

  protected:
    virtual void tickle();

    virtual void run();

    virtual void idle();

  private:
    template<class CorF>
    bool scheduleNonLock(CorF val,uint64_t thread){
      //队列中是否有任务需要处理
      bool needToTickle = m_corfs.empty();
      CorFType cf(val,thread);
      if(cf.m_coroutine || cf.m_func){
        res = true;
        m_corfs.push_back(cf);
      }
      return needToTickle;
    }

    //既可以传入协程，也可以传入函数,让函数以协程的方式调用
    class CorFType{
      public:
        CorFType(std::function<void()> func,uint64_t thread)
        :m_func(func)
        ,m_threadId(thread){

        }
        CorFType(gaiya::Coroutine::ptr coroutine,uint64_t thread)
        :m_coroutine(coroutine)
        ,m_threadId(thread){

        }
      public:
        std::function<void()> m_func;
        uint64_t m_threadId;
        gaiya::Coroutine::ptr m_coroutine;
    };
  private:
    //调度器名称
    std::string m_name;
    //线程池
    std::vector<gaiya::Thread::ptr> m_tpool;
    //协程函数队列
    std::list<CorFType> m_corfs;
    //普通互斥锁
    MuteType m_mutex;
  protected:
    uint64_t m_threadSize;
};
}

#endif
