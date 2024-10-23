#ifndef __LOCK_H__
#define __LOCK_H__
#include"Noncopyable.hpp"
#include<semaphore.h>
#include<stdexcept>
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

//互斥量模板类
template<class T>
class ScopeLockM{
  public:
    ScopeLockM(T& mutex):m_mutex(mutex){
      m_mutex.lock();
      m_isLock = true;
    }
    ~ScopeLockM(){
      unlock();
    }
    lock(){
      if(!m_isLock){
        m_mutex.lock();
        m_isLock = true;
      }
    }
    unlock(){
      if(m_isLock){
        m_mutex.unlock();
        m_isLock = false;
      }
    }
  private:
    T& m_mutex;
    bool m_isLock;
};

template<class T>
class ScopeReadLockM{
  public:
    ScopeLockM(T& mutex):m_mutex(mutex){
      m_mutex.rdlock();
      m_isLock = true;
    }
    ~ScopeLockM(){
      unlock();
    }
    lock(){
      if(!m_isLock){
        m_mutex.rdlock();
        m_isLock = true;
      }
    }
    unlock(){
      if(m_isLock){
        m_mutex.unlock();
        m_isLock = false;
      }
    }
  private:
    T& m_mutex;
    bool m_isLock;
};

template<class T>
class ScopeWriteLockM{
  public:
    ScopeLockM(T& mutex):m_mutex(mutex){
      m_mutex.wrlock();
      m_isLock = true;
    }
    ~ScopeLockM(){
      unlock();
    }
    lock(){
      if(!m_isLock){
        m_mutex.wrlock();
        m_isLock = true;
      }
    }
    unlock(){
      if(m_isLock){
        m_mutex.unlock();
        m_isLock = false;
      }
    }
  private:
    T& m_mutex;
    bool m_isLock;
};

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

class SpinMutex : Noncopyable {
public: 
    /// 局部锁
    typedef ScopeLockM<Mutex> Lock;

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


}
#endif
