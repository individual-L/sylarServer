#include<iostream>
#include"log.hpp"
#include"util.hpp"
#include"config.hpp"
#include<yaml-cpp/yaml.h>


int main(int argc, char** argv){
  try {
      LOG_INFO(LOG_ROOT()) << "start test config";
      gaiya::Config::loadYamlFile("../tester/log.yaml");
      LOG_INFO(LOG_ROOT()) << "end test config";
  } catch (const YAML::BadFile& e) {
      std::cerr << "Error opening file: " << e.what() << std::endl;
  } catch (const YAML::Exception& e) {
      std::cerr << "Error parsing YAML file: " << e.what() << std::endl;
  }
  return 1;
}
