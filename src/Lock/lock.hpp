#ifndef __LOCK_H__
#define __LOCK_H__
#include"Noncopyable.hpp"
#include<semaphore.h>
#include<stdexcept>
#include<atomic>
#include<thread>

namespace gaiya{



class Semaphore :public Noncopyable{
  public:
    Semaphore(uint32_t count = 0);
    ~Semaphore();

    void wait();

    void post();

  private:
    sem_t m_semaphore;
};

// 锁(互斥量，自旋锁) RAII模板类
template<class T>
class ScopeLockM{
  public:
    //构造函数加锁
    ScopeLockM(T& mutex):m_mutex(mutex){
      lock();
    }
    //析构函数释放锁
    ~ScopeLockM(){
      unlock();
    }
    void lock(){
      if(!m_isLock){
        m_mutex.lock();
        m_isLock = true;
      }
    }
    void unlock(){
      if(m_isLock){
        m_mutex.unlock();
        m_isLock = false;
      }
    }
  private:
    T& m_mutex;
    bool m_isLock = false;
};

// 读锁 RAII模板类
template<class T>
class ScopeReadLockM{
  public:
    ScopeReadLockM(T& mutex):m_mutex(mutex){
      lock();
    }
    ~ScopeReadLockM(){
      unlock();
    }
    void lock(){
      if(!m_isLock){
        m_mutex.rdlock();
        m_isLock = true;
      }
    }
    void unlock(){
      if(m_isLock){
        m_mutex.unlock();
        m_isLock = false;
      }
    }
  private:
    T& m_mutex;
    bool m_isLock = false;
};

// 写锁 RAII模板类
template<class T>
class ScopeWriteLockM{
  public:
    ScopeWriteLockM(T& mutex):m_mutex(mutex){
      lock();
    }
    ~ScopeWriteLockM(){
      unlock();
    }
    void lock(){
      if(!m_isLock){
        m_mutex.wrlock();
        m_isLock = true;
      }
    }
    void unlock(){
      if(m_isLock){
        m_mutex.unlock();
        m_isLock = false;
      }
    }
  private:
    T& m_mutex;
    bool m_isLock = false;
};
//互斥量
class Mutex : Noncopyable {
public: 
    /// 局部锁
    typedef ScopeLockM<Mutex> Lock;

    Mutex() {
        pthread_mutex_init(&m_mutex, nullptr);
    }

    ~Mutex() {
        pthread_mutex_destroy(&m_mutex);
    }

    void lock() {
        pthread_mutex_lock(&m_mutex);
    }

    void unlock() {
        pthread_mutex_unlock(&m_mutex);
    }
private:
    pthread_mutex_t m_mutex;
};

//读写锁
class RWMutex : Noncopyable {
public: 
    /// 局部锁
    typedef ScopeWriteLockM<RWMutex> WriteLock;
    typedef ScopeReadLockM<RWMutex> ReadLock;

    RWMutex() {
        pthread_rwlock_init(&m_mutex, nullptr);
    }

    ~RWMutex() {
        pthread_rwlock_destroy(&m_mutex);
    }

    void wrlock() {
        pthread_rwlock_wrlock(&m_mutex);
    }

    void rdlock() {
        pthread_rwlock_rdlock(&m_mutex);
    }

    void unlock() {
        pthread_rwlock_unlock(&m_mutex);
    }
private:
    pthread_rwlock_t m_mutex;
};

//自旋锁
class SpinMutex : Noncopyable {
public: 
    /// 局部锁
    typedef ScopeLockM<SpinMutex> Lock;

    SpinMutex() {
        pthread_spin_init(&m_mutex, 0);
    }

    ~SpinMutex() {
        pthread_spin_destroy(&m_mutex);
    }

    void lock() {
        pthread_spin_lock(&m_mutex);
    }

    void unlock() {
        pthread_spin_unlock(&m_mutex);
    }
private:
    pthread_spinlock_t m_mutex;
};

class CASMutex :public Noncopyable{
  public:
    typedef ScopeLockM<CASMutex> Lock;
    CASMutex(){
      //设为false
      m_mutex.clear();
    }
    ~CASMutex(){

    }
    void lock(){
      while(m_mutex.test_and_set(std::memory_order_acquire)){}
    }
    void unlock(){
      m_mutex.clear(std::memory_order_release);
    }
  private:
    std::atomic_flag m_mutex;
};

}
#endif
