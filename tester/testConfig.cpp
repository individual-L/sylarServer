#include<iostream>
#include"log.hpp"
#include"util.hpp"
#include"config.hpp"
#include<yaml-cpp/yaml.h>

void testVectoString(){
    gaiya::ConfigVar<std::vector<int>>::ptr tmp =  
    gaiya::Config::lookup("vec","system.vec",std::vector<int>{1,2});
    LOG_INFO(LOG_ROOT()) << tmp->getTypeName();
    std::fstream fs;
    fs.open("/home/luo/cplus/sylar/tester/configTest.yaml"
        ,std::ios::app);
    LOG_INFO(LOG_ROOT()) << tmp->toString();
    fs.flush();
    fs << tmp->toString() << std::endl;

}

void testSettoString(){
    gaiya::ConfigVar<std::set<std::string>>::ptr tmp =  
    gaiya::Config::lookup("set","system.vec",std::set<std::string>{"hello","nihao"});
    LOG_INFO(LOG_ROOT()) << tmp->getTypeName();
    std::fstream fs;
    fs.open("/home/luo/cplus/sylar/tester/configTest.yaml"
        ,std::ios::app);
    LOG_INFO(LOG_ROOT()) << tmp->toString();
    fs.flush();
    fs << tmp->toString() << std::endl;
}


int main(int argc, char** argv){
  try {
      LOG_INFO(LOG_ROOT()) << "start test config";
    //   gaiya::Config::loadYamlFile("../tester/log.yaml");
      testVectoString();
      testSettoString();
      LOG_INFO(LOG_ROOT()) << "end test config";
  } catch (const YAML::BadFile& e) {
      std::cerr << "Error opening file: " << e.what() << std::endl;
  } catch (const YAML::Exception& e) {
      std::cerr << "Error parsing YAML file: " << e.what() << std::endl;
  }
  return 1;
}
