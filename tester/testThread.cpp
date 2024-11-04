// #include"head.hpp"

// gaiya::Mutex mutex;
// int count = 0;

// void fun1(){
//   while(true){
//     LOG_DEBUG(LOG_GET_LOGGER("master")) <<"==============================";
//   }

// }


// void fun2(){
//   while(true){
//     LOG_DEBUG(LOG_GET_LOGGER("master")) <<"------------------------------";
//   }
// }


// int main(){
//   gaiya::Config::loadYamlFile("/home/luo/cplus/gaiya/tester/configTest.yaml");
//   std::cout<< "start" <<std::endl;
//   std::vector<gaiya::Thread::ptr> threads;
//   for(int i = 0;i < 2;++i){
//     threads.push_back(gaiya::Thread::ptr(new gaiya::Thread(&fun1,"name_" + std::to_string(i * 2 + 1))));
//     threads.push_back(gaiya::Thread::ptr(new gaiya::Thread(&fun2,"name_" + std::to_string(i + 1))));
//   }
//   for(uint64_t i = 0;i < threads.size();++i){
//     threads[i]->join();
//   }
//   LOG_INFO(LOG_GET_LOGGER("master")) << "test end thread";
//   return 1;
// }
