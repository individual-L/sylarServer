#include"util.hpp"

namespace gaiya{

// inline std::string getRelativePath(std::filesystem::path p){
//     // 文件路径：/home/luo/cplus
//     std::filesystem::path current_file = std::filesystem::current_path().parent_path() ; 

//     //目标文件
//     std::filesystem::path target_file = p; 

//     // 计算目标文件p与项目路径的相对路径
//     std::filesystem::path relative_path = target_file.lexically_relative(current_file.parent_path()); 
//     return relative_path;
// }


pid_t GetThreadId(){
  return syscall(SYS_gettid);
}

int32_t GetCoroutineId(){
  return 1;
}

}
