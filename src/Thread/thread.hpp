#ifndef __THREAD_H__

#define __THREAD_H__

#include<string>
#include<functional>
#include<memory>
#include"Noncopyable.hpp"
#include"lock.hpp"
#include"log.hpp"


namespace gaiya{

class Thread :public Noncopyable{
  public:
    typedef std::shared_ptr<Thread> ptr;

    Thread(const std::function<void()> cb,const std::string name);

    ~Thread();

    std::string getName()const {return m_name;}

    void setName(std::string name){m_name = name;}

    pid_t getId()const {return m_id;}
    //线程执行函数
    static void* run(void *);
    //设置当前线程名称
    static void SetName(std::string name);

    //获取当前线程名称
    static std::string GetName();

    //等待线程执行完成
    void join();

    //获取当前线程
    static Thread* GetThis();

  private:
    std::string m_name;
    pid_t m_id = -1;
    std::function<void()> m_cb;
    pthread_t m_thread = 0;

    Semaphore m_sem;
};



}

#endif
