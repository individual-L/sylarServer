#ifndef __COROUTINE_H__
#define __COROUTINE_H__

#include<ucontext.h>
#include<functional>
#include<memory>
#include<atomic>

#include"util.hpp"
#include"log.hpp"
#include"config.hpp"
#include"macro.hpp"

namespace gaiya{
class Scheduler;
class Coroutine :public std::enable_shared_from_this<Coroutine>{
  public:
    friend class Scheduler;
    typedef std::shared_ptr<Coroutine> ptr;
    enum State{
      //初始化
      INIT,
      //就绪状态
      READY,
      //执行中
      EXECU,
      //结束状态
      END,
      //阻塞状态
      HOLD,
      //异常状态
      EXCEPT
    };
  public:
  /*
  retnSche 执行完后是否将执行权让给调度协程(反之让给协程所在的线程的主协程)
  */
    Coroutine(std::function<void()> func,size_t stacksize = 0,bool retnSche = false);
    ~Coroutine();

    //重设写成函数，并重置状态
    void reset(std::function<void()> func);
    //将当前协程切换到执行状态
    void swapIn();
    //将当前协程切换到后台
    void swapOut();

    //将当前线程切换到执行状态，执行的此线程的执行协程
    void call();
    //将当前线程切换到后台状态，让给调度协程
    void back();

    size_t getId() const {return m_id;};

    State getState() const {return m_state;}; 

    public:
      static void MainFunc();

      static void SetCurCoro(Coroutine * ptr);

      static Coroutine::ptr GetCurCoro();

      static void YieldToReady();

      static void YieldToHold();
      
      static uint64_t GetCoroCount();

      static uint64_t GetCurCoroId();

  private:
    Coroutine();
    //协程id
    uint64_t m_id = 0;
    //协程函数
    std::function<void()> m_func;
    //协程上下文
    ucontext_t m_context;
    //栈大小
    uint64_t m_stackSize = 0;
    //协程栈指针
    void* m_stack = nullptr;
    //协程状态
    State m_state = INIT;


};

}

#endif
