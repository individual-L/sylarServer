#include"util.hpp"
#include"coroutine.hpp"
#include"thread.hpp"

namespace gaiya{

gaiya::Logger::ptr logger = LOG_ROOT();

void backTrace(std::vector<std::string>& bt,const int size,const int skip){
  void ** buff = (void **) malloc(sizeof(void*) * size);
  int n = ::backtrace(buff,size);
  char ** strings = ::backtrace_symbols(buff,n);

  if(strings == nullptr){
    LOG_ERROR(logger) << "backtrace_symbols error";
    std::cerr << "backtrace_symbols";
  }
  for(int i = skip;i < n;++i){
    bt.push_back(strings[i]);
  }
  free(buff);
  free(strings);
}

std::string backTraceToString(const int size,const int skip,const std::string& prefix){
  std::vector<std::string> bt;
  backTrace(bt,size,skip);
  std::stringstream ss;
  ss <<std::endl;
  for(auto& it : bt){
    ss << prefix << it <<std::endl;
  }
  return ss.str();
}


pid_t GetThreadId(){
  return syscall(SYS_gettid);
}

int32_t GetCoroutineId(){
  return gaiya::Coroutine::GetCurCoroId();
}

std::string GetThreadName(){
  return gaiya::Thread::GetName();
}

//删除path文件
bool Unlink(std::string path){
  return ::unlink(path.c_str()) == 0;
}

//非保留字符map
//-.0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz~
//上述字符对应的ascall码为下标
static const unsigned char reservedChar[256] = {
    /* 0 */
    0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 1, 1, 0,
    1, 1, 1, 1, 1, 1, 1, 1,   1, 1, 0, 0, 0, 1, 0, 0,
    /* 64 */
    0, 1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 0, 0, 0, 0, 1,
    0, 1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 0, 0, 0, 1, 0,
    /* 128 */
    0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
    /* 192 */
    0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
};


static const unsigned char ChartoHex[256] = {
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,1,2,3,4,5,6,7,8,9,0,0,0,0,0,0,
    0,10,11,12,13,14,15,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,10,11,12,13,14,15,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};

#define IS_RESERVE_CHAR(c) (reservedChar[(unsigned char)c])

std::string StringUrl::UrlEncode(const std::string& str, bool space_as_plus){
  const char hexChars[] = "0123456789ABCDEF";

  //当没有非保留字符时，可以延迟分配和减少拷贝来提高性能
  std::string* res = nullptr;
  //str.length()不含'\0'
  const char * end = str.c_str() + str.length();

  for(const char* c = str.c_str(); c < end;++c){
    if(!IS_RESERVE_CHAR(*c)){
      if(!res){
        res = new std::string();
        res->reserve(str.size() * 1.25);
        res->append(str.c_str(),c - str.c_str());
      }
      if(*c == ' ' && space_as_plus) {
          res->append(1, '+');
      } else {
          res->append(1, '%');
          res->append(1, hexChars[(uint8_t)*c >> 4]);
          res->append(1, hexChars[*c & 0x0f]);
      }  
    }else if(res){
      res->append(1, *c);
    }
  }
  if(!res){
    return str;
  } else {
      std::string rt = *res;
      delete res;
      return rt;
  }
}
void LoadConfigrationFile(){
  std::filesystem::path filePath(__FILE__);
  filePath = filePath.parent_path().parent_path();
  //如果以/为开头，他会把filePath设置为 /configTest.yaml
  filePath.append("configuration.yaml");
  gaiya::Config::loadYamlFile(filePath.c_str());
}

//处理按照url格式处理特殊字符的字符串
std::string StringUrl::UrlDecode(const std::string& str, bool space_as_plus){
  std::string* res = nullptr;
  const char* end = str.c_str() + str.length();

  for(const char* c = str.c_str(); c < end; ++c) {
    if(*c == '+' && space_as_plus) {
      //延迟分配
      if(!res) {
          res = new std::string;
          res->append(str.c_str(), c - str.c_str());
      }
      res->append(1, ' ');
    } else if(*c == '%' && (c + 2) < end
                && isxdigit(*(c + 1)) && isxdigit(*(c + 2))){
      if(!res) {
          res = new std::string;
          res->append(str.c_str(), c - str.c_str());
      }
      //将字符映射到相应的十六进制数字
      res->append(1, (char)((ChartoHex[(int)*(c + 1)]) << 4 | (ChartoHex[(int)*(c + 2)] & 0x0f)));
      c += 2;
    } else if(res) {
      res->append(1, *c);
    }
  }
  if(!res) {
      return str;
  } else {
      std::string rt = *res;
      delete res;
      return rt;
  }
}

std::string StringUrl::Trim(const std::string& str, const std::string& delimit){
    auto begin = str.find_first_not_of(delimit);
    if(begin == std::string::npos) {
        return "";
    }
    auto end = str.find_last_not_of(delimit);
    return str.substr(begin, end - begin + 1);
}

std::string TimeUtil::TimeToStr(time_t t,const char* format){
  struct tm tm;
  localtime_r(&t,&tm);
  std::string res;
  res.reserve(64);
  strftime(res.data(),res.length(),format,&tm);
  return res;
}
time_t TimeUtil::StrToTime(const char* str,const char* format){
  struct tm t;
  memset(&t, 0, sizeof(t));
  if(!strptime(str, format, &t)) {
      return 0;
  }
  return mktime(&t);
}

}
