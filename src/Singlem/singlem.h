
#ifndef __SINGLEM_H__
#define __SINGLEM_H__

#include<memory>


namespace gaiya {

//单例模式封装类
template<class T>
class Singlem {
  public:
    T * getInstance(){
      static T v;
      return &v;
    }
};
//单例模式智能指针封装类
template<class T>
class SinglemPtr {
  public:
    std::shared_ptr<T>  getInstance(){
      static std::shared_ptr<T> v(new T);
      return v;
    }
};
}


#endif
