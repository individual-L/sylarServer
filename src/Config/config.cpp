#include"config.hpp"

namespace gaiya{

static void listYamlFile(const std::string prefix,YAML::Node node
  , std::list<std::pair<std::string,YAML::Node>>& allist){

  if(prefix.find_first_not_of("qwertyuiopasdfghjklzxcvbnm_.1234567890") 
    != std::string::npos){
      LOG_ERROR(LOG_ROOT()) << "invalid argument: " << prefix <<": "<<node;
      throw std::invalid_argument(prefix);
  }
  allist.push_back(std::make_pair(prefix,node));
  if(node.IsSequence()){
    for(size_t i = 0;i < node.size();++i){
      listYamlFile(prefix,node[i],allist);
    }
  }
  if(node.IsMap()){
    for(auto it = node.begin();it != node.end();++it){
      listYamlFile(prefix.empty() ? it->first.Scalar() 
        : prefix + "." + it->first.Scalar(),it->second,allist);
    }
  }
  return;
}

ConfigVarBase::ptr Config::checkBase(const std::string& base){
  auto it = getDatas().find(base);
  if(it != getDatas().end()){
    return it->second;
  }
  return nullptr;
}


void Config::loadYamlFile(const char * file){
  std::list<std::pair<std::string,YAML::Node>> allist;
  YAML::Node node = YAML::LoadFile(file);
  listYamlFile("",node,allist);
  for(auto it : allist){
    std::string name = it.first;
    if(name.empty()){
      continue;
    }
    std::transform(name.begin(),name.end(),name.begin(),::tolower);

    gaiya::ConfigVarBase::ptr val = checkBase(name);

    if(val){
      if(it.second.IsScalar()){
        val->fromString(it.second.Scalar());
      }else{
        std::stringstream ss;
        ss<<it.second.Scalar();
        val->fromString(ss.str());
      }
    }

  }
}


}
