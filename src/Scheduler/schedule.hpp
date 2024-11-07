#ifndef __SCHEDULER_H__
#define __SCHEDULER_H__

#include<string>
#include<vector>
#include<list>


#include"thread.hpp"
#include"lock.hpp"
#include"coroutine.hpp"
#include"util.hpp"
#include"macro.hpp"
#include"log.hpp"


namespace gaiya{

class Scheduler : public std::enable_shared_from_this<Scheduler>{
  public:
  typedef std::shared_ptr<Scheduler> ptr;
  typedef gaiya::Mutex MuteType;
    Scheduler(uint64_t size = 1,bool useSche = true,const std::string& name = "");
    virtual ~Scheduler();

    const std::string getName() const {return m_name;}
    //获取协程调度器
    static Scheduler::ptr GetThis();
    //设置协程调度器为自己
    void setThis();

    //获取调度协程
    static gaiya::Coroutine::ptr GetMasterCoro();

    //初始化线程池
    void start();
    //暂停线程池的运转
    void stop();


    //添加协程或者函数
    template<class CorF>
    void schedule(CorF val,uint64_t thread = -1){
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
          needToTickle = m_coros.push_back(CorFType(*begin,-1));
          ++begin;
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

    virtual bool stopping();

  private:
    template<class CorF>
    bool scheduleNonLock(CorF val,uint64_t thread){
      //队列中是否有任务需要处理
      bool needToTickle = m_coros.empty();
      CorFType cf(val,thread);
      if(cf.m_coroutine || cf.m_func){
        res = true;
        m_coros.push_back(cf);
      }
      return needToTickle;
    }

    //既可以传入协程，也可以传入函数,让函数以协程的方式调用
    class CorFType{
      public:
        CorFType(){}
        CorFType(std::function<void()> func,uint64_t thread)
        :m_func(func)
        ,m_threadId(thread){

        }
        CorFType(gaiya::Coroutine::ptr coro,uint64_t thread)
        :m_coro(coro)
        ,m_threadId(thread){

        }
        void reset(){
          m_func = nullptr;
          m_threadId = -1;
          m_coro = nullptr;
        }
      public:
        std::function<void()> m_func;
        //协程指定运行的id
        uint64_t m_threadId;
        gaiya::Coroutine::ptr m_coro;
    };
  private:
    //调度器名称
    std::string m_name;
    //线程池
    std::vector<gaiya::Thread::ptr> m_tpool;
    //协程函数队列
    std::list<CorFType> m_coros;
    //普通互斥锁
    MuteType m_mutex;
    //调度协程
    gaiya::Coroutine::ptr m_masterCoro;
    //调度协程所在的线程id
    uint32_t m_masterTid;
  protected:
    //线程数量
    uint32_t m_threadCount;

    //协程下的线程id数组
    std::vector<uint32_t> m_threadIds;
    //活跃线程数量
    std::atomic<uint32_t> m_activeThreadCount = {0};
    //空闲线程数量
    std::atomic<uint32_t> m_idleThreadCount = {0};
    //是否处于激活状态
    bool m_active = false;
    //是否自动关闭
    bool m_autoStop = false;

};
}

#endif
