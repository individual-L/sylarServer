#ifndef __CONFIG_H__
#define __CONFIG_H__

#include<memory>
#include<sstream>
#include<boost/lexical_cast.hpp>
#include<log.h>

namespace gaiya{

class ConfigVarBase{
  public:
  typedef std::shared_ptr<ConfigVarBase> ptr;
    ConfigVarBase(const std::string name,const std::string description)
      :m_name(name)
      ,m_description(description){
      }
    virtual ~ConfigVarBase(){}
    std::string getName() const {return m_name;}
    std::string getDescription() const {return m_description;}

    virtual std::string toString() = 0;
    virtual bool fromString(const std::string& str) = 0;
  protected:
    std::string m_name;
    std::string m_description;
};

template<class T>
class ConfigVar :public ConfigVarBase{
  public:
    ConfigVar(const std::string name,const std::string description
    ,const T& default_val)
    :ConfigVarBase(name,description)
    ,m_val(default_val){

    }
    std::string toString() override {
      try
      {
        return boost::lexical_cast<std::string>(m_val);
      }
      catch(const std::exception& e)
      {
        LOG_ERROR(LOG_ROOT())<<"ConfigVar::toString exception" << e.what()
        << "convert" << typeid(m_val).name << " to string";
      }
      return "";
    }
    bool fromString(const std::string& str) override {
      try
      {
        m_val = boost::lexical_cast<T>(str);
      }
      catch(const std::exception& e)
      {
        LOG_ERROR(LOG_ROOT())<<"ConfigVar::toString exception" << e.what()
        << "convert string(" << str << ") to " << typeid(m_val).name;
      }
      return false;
    }
  private:
    T m_val;
};


}

#endif
