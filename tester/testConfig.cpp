#include<iostream>
#include"log.hpp"
#include"log.cpp"
#include"util.hpp"
#include"config.hpp"
#include<yaml-cpp/yaml.h>


void testFunc(){
    gaiya::ConfigVar<int>::ptr config_int =  gaiya::Config::lookup("system.port","端口号",int(9090));

    gaiya::ConfigVar<std::string>::ptr config_string =  gaiya::Config::lookup("system.ip","ip地址",std::string("127.0.0.1"));

    gaiya::ConfigVar<std::vector<int>>::ptr config_vec =  gaiya::Config::lookup("system.vector","vecotr",std::vector<int>{1,2,3});

    gaiya::ConfigVar<std::set<int>>::ptr config_set =  gaiya::Config::lookup("system.set","set",std::set<int>{4,5,6,7});

    gaiya::ConfigVar<std::map<std::string,int>>::ptr config_map =  gaiya::Config::lookup("system.map","map",std::map<std::string,int>
    {{"ip",127},{"port",8989}});
    gaiya::ConfigVar<std::map<std::string,gaiya::Person>>::ptr config_person = gaiya::Config::lookup("system.person","class person",std::map<std::string,gaiya::Person>{{"nihao",{}}});

    gaiya::ConfigVar<std::map<std::string,std::vector<gaiya::Person>>>::ptr config_vec_person = gaiya::Config::lookup("system.vec_map","vec_map"
    ,std::map<std::string,std::vector<gaiya::Person>>{});

    config_person->addCallBackFunc([](std::map<std::string,gaiya::Person> oldDatas,std::map<std::string,gaiya::Person> newDatas)->void{
        LOG_INFO(LOG_ROOT())<<"oldDatas: " << oldDatas.size() <<" newdata: " << newDatas.size();
    }
    );


#define XX(name,val,change) \
    { \
    auto v = val->getValue(); \
    for(auto& it : v){ \
        LOG_INFO(LOG_ROOT())<< #name << " " << #change <<" " << it ; \
    } \
    LOG_INFO(LOG_ROOT())<< #name << " " << #change<<std::endl << val->toString(); \
    }

#define XX_MAP(name,val,change) \
    { \
    auto v = val->getValue(); \
    for(auto& it : v){ \
        LOG_INFO(LOG_ROOT())<< #name << " " << #change <<" " << it.first <<": "<<it.second ; \
    } \
    LOG_INFO(LOG_ROOT())<< #name << " " << #change<<std::endl<< val->toString(); \
    }
#define XX_Person(name,val,change) \
    { \
    auto v = val->getValue(); \
    for(auto& it : v){ \
        LOG_INFO(LOG_ROOT())<< #name << " " << #change <<" " << it.first <<": "<< it.second.toString(); \
    } \
    LOG_INFO(LOG_ROOT())<< #name << " " << #change<<std::endl<< val->toString(); \
    }
#define XX_Vec_Person(name,val,change) \
    { \
    auto v = val->getValue(); \
    for(auto& it : v){ \
        LOG_INFO(LOG_ROOT())<< #name << " " << #change <<" " << it.first <<": "; \
        for(auto j : it.second){ \
            LOG_INFO(LOG_ROOT()) << j.toString(); \
        } \
    } \
    LOG_INFO(LOG_ROOT())<< #name << " " << #change<<std::endl<< val->toString(); \
    }


    try {
        LOG_INFO(LOG_ROOT()) << "start test config";
        
        XX(system.vec,config_vec,before);
        XX(system.set,config_set,before);
        XX_MAP(system.map,config_map,before);
        XX_Person(system.person,config_person,before);
        XX_Vec_Person(system.vec_person,config_vec_person,before);
        gaiya::Config::loadYamlFile
            ("/home/luo/cplus/sylar/tester/configTest.yaml");
            
        XX(system.vec,config_vec,after);
        XX(system.set,config_set,after);
        XX_MAP(system.map,config_map,after);
        XX_Person(system.person,config_person,after);
        XX_Vec_Person(system.vec_person,config_vec_person,after);
        LOG_INFO(LOG_ROOT()) << "end test config";
    } catch (const YAML::BadFile& e) {
        std::cerr << "Error opening file: " << e.what() << std::endl;
    } catch (const YAML::Exception& e) {
        std::cerr << "Error parsing YAML file: " << e.what() << std::endl;
    }
}

void testLogConfig(){
    
    lOG_INFO_ROOT() <<std::endl << gaiya::s_LoggersM::getInstance()->toYamlString();
    gaiya::Config::loadYamlFile("/home/luo/cplus/sylar/tester/configTest.yaml");


    LOG_INFO(LOG_GET_LOGGER("master")) <<std::endl << gaiya::s_LoggersM::getInstance()->toYamlString();

    LOG_M()->getLogger("master")->setFormater("%d%T%ln%T%p%T%t%T%F%T%f%T%l%T%m%n");
    
    LOG_INFO(LOG_GET_LOGGER("master")) <<std::endl << gaiya::s_LoggersM::getInstance()->toYamlString();
}

int main(int argc, char** argv){
    // testFunc();
    testLogConfig();
    return 1;
}
