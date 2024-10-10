#include<iostream>
#include"log.h"
#include"util.h"
#include"config.h"
#include<yaml-cpp/yaml.h>
void print_yaml(const YAML::Node& node,int level){
  if(node.IsScalar()){
    LOG_INFO(LOG_ROOT())<<std::string(level * 4,' ') <<node.Type() << ": " << node.Scalar();
  }else if(node.IsNull()){
    LOG_INFO(LOG_ROOT())<< std::string(level*4,' ') << node.Type() << ": null";
  }else if(node.IsMap()){
    for(auto it = node.begin();it != node.end();++it ){
    LOG_INFO(LOG_ROOT())<< std::string(level * 4,' ') << node.Type() << " - " 
    << it->first << ": ";
    print_yaml(it->second,level + 1);
    }
  }else if(node.IsSequence()){
    for(size_t i = 0;i < node.size();++i){
      LOG_INFO(LOG_ROOT())<< std::string(level * 4,' ') << node.Type() << " - ";
      print_yaml(node[i],level + 1);
    }
  }
}
void parse_yaml(YAML::Node node,int level){
  print_yaml(node,level);
}
int main(int argc, char** argv){
  std::cout << "start test config" << std::endl;
  try {
      YAML::Node config = YAML::LoadFile("../tester/log.yaml");
      parse_yaml(config,0);
  } catch (const YAML::BadFile& e) {
      std::cerr << "Error opening file: " << e.what() << std::endl;
  } catch (const YAML::Exception& e) {
      std::cerr << "Error parsing YAML file: " << e.what() << std::endl;
  }
  std::cout << "end test config" << std::endl;
  return 1;
}
