
#ifndef __SINGLEM_H__
#define __SINGLEM_H__

#include<memory>


namespace gaiya {

//单例模式封装类
template<class T>
class Singlem {
  public:
    static T * getInstance(){
      if(!instance){
        instance = new T();
      }
      return &instance;
    }
  private:
    static T* instance;
};
//单例模式智能指针封装类
template<class T>
class SinglemPtr {
  public:
    static std::shared_ptr<T>  getInstance(){
      if(!instance){
        instance =  std::make_shared<T>();
      }
      return instance;
    }
  private:
    static std::shared_ptr<T> instance;
};
template<class T>
std::shared_ptr<T> SinglemPtr<T>::instance = nullptr;

template<class T>
T* Singlem<T>::instance = nullptr;





}
#endif
