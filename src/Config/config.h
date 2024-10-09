#ifndef __CONFIG_H__
#define __CONFIG_H__

#include<memory>
#include<sstream>
#include<boost/lexical_cast.hpp>
#include"log.h"
#include<map>
#include<yaml-cpp/yaml.h>

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
    virtual std::string getTypeName() = 0;
    virtual std::string toString() = 0;
    virtual bool fromString(const std::string& str) = 0;
  protected:
    std::string m_name;
    std::string m_description;
};

template<class T>
class ConfigVar :public ConfigVarBase{
  public:
    typedef std::shared_ptr<ConfigVar<T> > ptr;
    ConfigVar(const std::string name,const std::string description
    ,const T& default_val)
    :ConfigVarBase(name,description)
    ,m_val(default_val){

    }

    const T getValue() const { return m_val; }

    std::string toString() override {
      try
      {
        return boost::lexical_cast<std::string>(m_val);
      }
      catch(const std::exception& e)
      {
        LOG_ERROR(LOG_ROOT())<<"ConfigVar::toString exception" << e.what()
        << "convert" << typeid(m_val).name() << " to string";
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
        << "convert string(" << str << ") to " << typeid(m_val).name();
      }
      return false;
    }

    std::string getTypeName() override {
      return gaiya::TypeName<T>();
    }

  private:
    T m_val;
};
class Config{
  public:
    typedef std::map<std::string,gaiya::ConfigVarBase::ptr> configVarMap;
    
    template<class T>
    static typename ConfigVar<T>::ptr lookup(const std::string name,const std::string description,T& val){
      auto it = getDatas().find(name);
      if(it != getDatas().end()){
        auto v = std::dynamic_pointer_cast<ConfigVar<T> >(it->second);
        if(v){
          LOG_INFO(LOG_ROOT())<< "lookup type " << name <<" exist";
          return v;
        }else{
          LOG_ERROR(LOG_ROOT())<< "lookup name "<<name <<" exist but type not " << TypeName<T>() <<  " map_type is "<< it->second->getTypeName();
          return nullptr;
        }
      }

      if(name.find_first_not_of("qwertyuiopasdfghjklzxcvbnm._1234567890")
       != std::string::npos){
        throw std::invalid_argument(name);
      }
      typename ConfigVar<T>::ptr v(new ConfigVar<T>(name,description,val));
      getDatas()[name] = v;
      return v;
    }
    template<class T>
    static typename ConfigVar<T>::ptr lookup(const std::string name){
      auto it = getDatas().find(name);
      if(it != getDatas().end()){
        auto v = std::dynamic_pointer_cast<ConfigVar<T> >(it->second);
        if(v){
          LOG_INFO(LOG_ROOT())<< "lookup type " << name <<" exist";
          return v;
        }else{
          LOG_ERROR(LOG_ROOT())<< "lookup name "<<name <<" exist but type not " << TypeName<T>() <<  " map_type is "<< it->second->getTypeName();
          return nullptr;
        }
      }
      return nullptr;
    }
  private:
    static configVarMap& getDatas(){
      static configVarMap s_datas;
      return s_datas;
    }
};

}

#endif
