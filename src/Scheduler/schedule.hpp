#ifndef __SCHEDULER_H__
#define __SCHEDULER_H__

#include<string>
#include<vector>
#include<list>


#include"thread.hpp"
#include"lock.hpp"
#include"coroutine.hpp"

namespace gaiya{

class Scheduler : public std::enable_shared_from_this<Scheduler>{
  public:
  typedef std::shared_ptr<Scheduler> ptr;
  typedef gaiya::Mutex MuteType;
    Scheduler(uint64_t size = 1,bool useSche = true,const std::string& name = "main");
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
    //开启自动关闭，当线程池无活跃线程时，任务队列无任务时，开启自动关闭时就关闭idle协程
    void stop();


    //添加协程或者函数
    template<class CorF>
    void schedule(CorF&& val,const uint64_t thread = -1){
      //队列中是否有任务需要处理
      bool needToTickle = false;
    {
      MuteType::Lock lock(m_mutex);
      needToTickle = scheduleNonLock(std::forward<CorF>(val),thread);
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
          needToTickle = scheduleNonLock(*begin,-1) || needToTickle;
          ++begin;
        }
      }
      if(needToTickle){
        tickle();
      }
    } 

  protected:
    virtual void tickle();

    void run();

    virtual void idle();

    virtual bool stopping();

    bool hasThread(){return m_idleThreadCount > 0;};

  private:
    template<class CorF>
    bool scheduleNonLock(CorF&& val,const uint64_t thread){
      //队列是否为空
      bool needToTickle = m_coros.empty();
      CorFType cf(std::forward<CorF>(val),thread);
      if(cf.m_coro || cf.m_func){
        m_coros.push_back(cf);
      }
      return needToTickle;
    }

    //既可以传入协程，也可以传入函数,让函数以协程的方式调用
    class CorFType{
      public:
        CorFType():m_threadId(-1){}
        CorFType(std::function<void()>& func,uint64_t thread)
        :m_threadId(thread)
        ,m_func(func){

        }
        CorFType(gaiya::Coroutine::ptr& coro,uint64_t thread)
        :m_threadId(thread)
        ,m_coro(coro){

        }
        CorFType(std::function<void()>&& func,uint64_t thread)
        :m_threadId(thread)
        ,m_func(func){

        }
        ~CorFType(){
          m_func = nullptr;
          m_threadId = -1;
          m_coro = nullptr;
        }
        void reset(){
          m_func = nullptr;
          m_threadId = -1;
          m_coro = nullptr;
        }
      public:
        int m_threadId = -1;
        std::function<void()> m_func;
        //协程指定运行的id
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
    int m_masterTid;
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
    bool m_stopping = true;
    //是否自动关闭
    bool m_autoStop = false;

};
}

#endif
