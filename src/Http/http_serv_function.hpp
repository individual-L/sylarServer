#ifndef __HTTP_SERV_FUNCTION_H__
#define __HTTP_SERV_FUNCTION_H__

#include<memory>
#include<string>
#include"http.hpp"
#include"http_session.hpp"
#include<functional>
#include"lock.hpp"
#include"util.hpp"
#include"singlem.hpp"

namespace gaiya{

//根据http请求调用相应的处理函数初始化http响应---基类
class ServBase {
public:
  typedef std::shared_ptr<ServBase> ptr;
  ServBase(){}
  ServBase(const std::string& name):m_name(name) {}

  virtual ~ServBase() {}

  virtual void handle(gaiya::http::HttpRequest::ptr request
                  , gaiya::http::HttpResponse::ptr response) = 0;
                  
  const std::string& getName() const { return m_name;}
protected:
    /// 名称
    std::string m_name;
};

//根据http请求调用回调函数初始化http响应
class ServCallBack: public ServBase {
public:
  typedef std::shared_ptr<ServCallBack> ptr;
  typedef std::function<void(gaiya::http::HttpRequest::ptr request
                  , gaiya::http::HttpResponse::ptr response)> TypeCallBack;
  ServCallBack(){};
  ServCallBack(TypeCallBack cb);

  void setCallBack(TypeCallBack cb){m_cb = cb;};

  void handle(gaiya::http::HttpRequest::ptr request
                  , gaiya::http::HttpResponse::ptr response 
                  ) override;
private:
  TypeCallBack m_cb;
};

//工厂基类
class OriServCreator {
public:
    typedef std::shared_ptr<OriServCreator> ptr;
    virtual ~OriServCreator() {}
    virtual ServBase::ptr get() const = 0;
    virtual std::string getName() const = 0;
};

//具体参数使用，使用工厂
class ServHoldCreator :public OriServCreator{
  public:
    typedef std::shared_ptr<ServHoldCreator> ptr;
    ServHoldCreator(ServBase::ptr val);
    ~ServHoldCreator() {}
    ServBase::ptr get() const override{
      return m_serv;
    }
    std::string getName() const override{
      return m_serv->getName();
    };
  private:
    ServBase::ptr m_serv;
};

//指导具体类型使用，生产工厂
template<class T>
class ServCreator : public OriServCreator{
public:
    ServBase::ptr get() const override{
      return ServBase::ptr(new T());
    }
  std::string getName() const override{
    return TypeName<T>();
  };
};

//根据相应的url链接进行匹配，调用所对应的处理函数
class ServDispatch :public ServBase{
  public:
    typedef std::shared_ptr<ServDispatch> ptr;
    typedef gaiya::RWMutex TypeRWMutex;
    ServDispatch();
    void handle(gaiya::http::HttpRequest::ptr request
                   , gaiya::http::HttpResponse::ptr response
                   ) override;
    
    //
    void addServBase(const std::string& uri, ServBase::ptr serv);

    void addServBase(const std::string& uri, ServCallBack::TypeCallBack cb);

    void addGlobServBase(const std::string& uri, ServBase::ptr serv);

    void addGlobServBase(const std::string& uri, ServCallBack::TypeCallBack cb);

    void delServBase(const std::string& uri);

    void delGlobServBase(const std::string& uri);

    ServBase::ptr getDefault() const { return m_default;}

    void setDefault(ServBase::ptr v) { m_default = v;}

    ServBase::ptr getServBase(const std::string& uri);

    ServBase::ptr getGlobServBase(const std::string& uri);

    ServBase::ptr getMatchedServBase(const std::string& uri);
  private:
    // 读写互斥量
    TypeRWMutex m_mutex;
    // 精准匹配 uri(/root/xxx) -> serv
    std::unordered_map<std::string, OriServCreator::ptr> m_datas;
    // 模糊匹配 uri(/root/*) -> serv
    std::vector<std::pair<std::string, OriServCreator::ptr> > m_globs;
    // 默认serv，所有路径都没匹配到时使用
    ServBase::ptr m_default;
};

typedef gaiya::SinglemPtr<ServDispatch> s_servDispatch;


//404
class ServDefault : public ServBase{
  public:
    ServDefault();
    ServDefault(std::string& content);
    void handle(gaiya::http::HttpRequest::ptr request
          , gaiya::http::HttpResponse::ptr response
          ) override ;
  private:
    std::string m_content;
};
}


#endif
