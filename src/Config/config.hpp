#ifndef __CONFIG_H__
#define __CONFIG_H__

#include<memory>
#include<sstream>
#include<boost/lexical_cast.hpp>
#include"log.hpp"
#include<map>
#include<unordered_map>
#include<set>
#include<unordered_set>
#include<yaml-cpp/yaml.h>
#include<utility>
#include"util.hpp"

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
    virtual std::string getTypeName()const = 0;
    virtual std::string toString() = 0;
    virtual bool fromString(const std::string& str) = 0;
  protected:
    std::string m_name;
    std::string m_description;
};
//S:source(源类型),G:目标类型(destination)
template<class S,class D>
class lexicalCast{
  public:
    void operator(const S& s)(){
      return boost::lexical_cast<D>(s);
    }
};

//类型转换模板类特化(YAML string ==> vecotr<T>)
template<class T>
class lexicalCast<std::string,std::vector<T>>{
  public:
    std::vecotr<T> operator(const std::string str)(){
      std::stringstream ss;
      typename std::vecotr<T> res;
      YAML::Node node = YAML::load(str);
      for(size_t i = 0;i < node.size();++i){
        ss.str(std::string());
        ss << node[i];
        res.push_back(lexicalCase<std::string,T>()(ss.str()));
      }
      return vec;
    }
};

//类型转换模板类特化(vecotr<T> ==> YAML string)
template<class T>
class lexicalCast<std::vector<T>,std::string>{
  public:
    void operator(std::vector<T> v)(){
      YAML::Node node(YAML::NodeType::Sequence);
      for(auto& it : v){
        node.push_back(YAML::load(lexicalCase<T,std::string>()(it)));
      }
      std::stringstream ss;
      ss << node;
      return ss.str();
    }
};

//类型转换模板类特化(YAML string ==> set<T>)
template<class T>
class lexicalCast<std::string,std::set<T>>{
  public:
    std::set<T> operator(const std::string str)(){
      typename std::set<T> res;
      YAML::Node node = YAML::load(str);
      std::stringstream ss;
      for(auto& it = node.begin();it != node.end();++it){
        ss.str(std::string());
        ss << *(it);
        res.insert(lexicalCast<std::string,T>()(ss.str()));
      }
      return res;
    }
};

//类型转换模板类特化(set<T> ==> YAML string)
template<class T>
class lexicalCast<std::set<T>,std::string>{
  public:
    std::string operator(std::set<T> s)(){
      YAML::Node node(YAML::NodeType::Sequence);
      for(auto& it : s){
        node.push_back(YAML::load(lexicalCase<T,std::string>()(it)));
      }
      std::stringstream ss;
      ss << node;
      return ss;
    }
};

//类型转换模板类特化(YAML string ==> map<std::string,T>)
template<class T>
class lexicalCast<std::string,std::map<std::string,T>>{
  public:
    std::map<std::string,T> operator(const std::string str)(){
      typename std::map<std::string,T> res;
      YAML::Node node = YAML::load(str);
      std::stringstream ss;
      for(auto& it = node.begin();it != node.end();++it){
        ss.str(std::string());
        ss << it->second;
        res.insert(std::map<std::string,T>::value_type(it->first.Scalar(),lexicalCast<std::string,T>()(ss.str())));
      }
      return res;
    }
};

//类型转换模板类特化(map<std::string,T> ==> YAML string)
template<class T>
class lexicalCast<std::map<std::string,T>,std::string>{
  public:
    std::string operator(std::map<std::string,T> m)(){
      YAML::Node node(YAML::NodeType::Map);
      for(auto& it : m){
        node[it.first] = YAML::load(lexicalCast<T,std::string>()(it.second));
      }
      std::stringstream ss;
      ss << node;
      return ss.str();
    }
};

//类型转换模板类特化(YAML string ==> unordered_map<std::string,T>)
template<class T>
class lexicalCast<std::unordered_map<std::string,T>,std::string>{
  public:
    std::unordered_map<std::string,T> operator(const std::string str)(){
      typename std::unordered_map<std::string,T> res;
      YAML::Node node = YAML::load(str);
      std::stringstream ss;
      for(auto& it = node.begin();it != node.end();++it){
        ss.str(std::string());
        ss << it->second;
        res.insert(std::unordered_map<std::string,T>::value_type(it->first.Scalar(),lexicalCast<std::string,T>()(ss.str())));
      }
      return res;
    }
};

//类型转换模板类特化(unordered_map<std::string,T> ==> YAML string)
template<class T>
class lexicalCast<std::string,std::unordered_map<std::string,T>>{
  public:
    std::string operator(std::unordered_map<std::string,T> m)(){
      YAML::Node node(YAML::NodeType::Map);
      for(auto& it : m){
        node[it.first] = YAML::load(lexicalCast<T,std::string>()(it.second));
      }
      std::stringstream ss;
      ss << node;
      return ss.str();
    }
};

//类型转换模板类特化(YAML string ==> unordered_set<T>)
template<class T>
class lexicalCast<std::string,std::unordered_set<T>>{
  public:
    std::unordered_set<T> operator(const std::string str)(){
      typename std::unordered_set<T> res;
      YAML::Node node = YAML::load(str);
      std::stringstream ss;
      for(auto& it = node.begin();it != node.end();++it){
        ss.str(std::string());
        ss << *(it);
        res.insert(lexicalCast<std::string,T>()(ss.str()));
      }
      return res;
    }
};

//类型转换模板类特化(unordered_set<T> ==> YAML string)
template<class T>
class lexicalCast<std::unordered_set<T>,std::string>{
  public:
    std::string operator(std::unordered_set<T> s)(){
      YAML::Node node(YAML::NodeType::Sequence);
      for(auto& it : s){
        node.push_back(YAML::load(lexicalCase<T,std::string>()(it)));
      }
      std::stringstream ss;
      ss << node;
      return ss;
    }
};

//类型转换模板类特化(YAML string ==> list<T>)
template<class T>
class lexicalCast<std::string,std::list<T>>{
  public:
    std::list<T> operator(const std::string str)(){
      std::stringstream ss;
      typename std::list<T> res;
      YAML::Node node = YAML::load(str);
      for(size_t i = 0;i < node.size();++i){
        ss.str(std::string());
        ss << node[i];
        res.push_back(lexicalCase<std::string,T>()(ss.str()));
      }
      return vec;
    }
};

//类型转换模板类特化(list<T> ==> YAML string)
template<class T>
class lexicalCast<std::list<T>,std::string>{
  public:
    void operator(std::list<T> v)(){
      YAML::Node node(YAML::NodeType::Sequence);
      for(auto& it : v){
        node.push_back(YAML::load(lexicalCase<T,std::string>()(it)));
      }
      std::stringstream ss;
      ss << node;
      return ss.str();
    }
};



template<class T,class FromStr = lexicalCast<std::string,T>
  ,class toStr = lexicalCast<T,std::string>>
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
        return toStr()(m_val);
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
        m_val = FromStr()(str);
        return true;
      }
      catch(const std::exception& e)
      {
        LOG_ERROR(LOG_ROOT())<<"ConfigVar::toString exception" << e.what()
        << "convert string(" << str << ") to " << typeid(m_val).name();
      }
      return false;
    }

    std::string getTypeName() const override {
      return gaiya::TypeName<T>();
    }
    void setVal(const T& val){m_val = val;}

  private:
    T m_val;
};
class Config{
  public:
    typedef std::map<std::string,gaiya::ConfigVarBase::ptr> configVarMap;
    typedef std::shared_ptr<Config> ptr;
    
    static void loadYamlFile(const char * file);

    static ConfigVarBase::ptr checkBase(const std::string& base);

    template<class T>
    static typename ConfigVar<T>::ptr lookup(const std::string& name,const std::string& description,T& val){
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
    static typename ConfigVar<T>::ptr lookup(const std::string& name){
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
  

    static void visit(std::function<void(ConfigVarBase::ptr)> func);
  private:
    static configVarMap& getDatas(){
      static configVarMap s_datas;
      return s_datas;
    }
};

}

#endif
