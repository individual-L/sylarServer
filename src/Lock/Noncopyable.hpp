#ifndef __NONCOPYABLE_H__

#define __NONCOPYABLE_H__
namespace gaiya{

class Noncopyable
{
private:
  /* data */
public:
  //禁用所有继承此类子类的复制赋值行为
  Noncopyable(/* args */) = default;
  Noncopyable(const Noncopyable &) = delete;
  Noncopyable& operator == (const Noncopyable& ) = delete;
  ~Noncopyable() = default;
};
  
}

#endif
