#include"log.h"
#include<tuple>
namespace sylar{

//LogLevel
const char * LogLevel::toString(const LogLevel::Level level){
  switch(level){
    #define XX(str)  \
    case LogLevel::str: \
      return #str; \
      break;
    XX(DEBUG)
    XX(INFO)
    XX(WARN)
    XX(ERROR)
    XX(FATAL)
    #undef XX
    default:
      return "UNKONW";
  }
  return "UNKONW";
}
LogLevel::Level LogLevel::toLevel(const std::string& str){
  #define XX(name,level)  \
  if(str == #name){     \
    return LogLevel::level; \
  }
  XX(DEBUG,DEBUG)
  XX(INFO,INFO)
  XX(WARN,WARN)
  XX(ERROR,ERROR)
  XX(FATAL,FATAL)
  return LogLevel::UNKNOW;
  #undef XX
}

//Logger
Logger::Logger(const std:: string name = "root"):m_name(name){}
void Logger::log (LogEvent::Level level,LogEvent::ptr event){
  if(level >= m_level){
    for(auto it : m_appenders){
      it->log(level,event);
    }
  }
}
void Logger::addAppenders(LogAppender::ptr appender){
  m_appenders.push_back(appender);
}
void Logger::delAppenders(LogAppender::ptr appender){
  for(auto it = m_appenders.begin();it != m_appenders.end();it++){
    if(*it == appender){
      m_appenders.erase(it);
      break;
    }
  }
}
void Logger::debug(LogEvent::ptr event){
  log(LogEvent::Level::DEBUG,event);
}
void Logger::info(LogEvent::ptr event){
  log(LogEvent::INFO::DEBUG,event);

}
void Logger::warn(LogEvent::ptr event){
  log(LogEvent::WARN::DEBUG,event);

}
void Logger::error(LogEvent::ptr event){
  log(LogEvent::ERROR::DEBUG,event);

}
void Logger::fatal(LogEvent::ptr event){
  log(LogEvent::FATAL::DEBUG,event);

}
void StdLogAppender::log(LogLevel::Level level,LogEvent::ptr event){
  if(level >= m_level){
    std::cout<<m_logFormater.format(event);
  }
}
void FileLogAppender::log(LogLevel::Level level,LogEvent::ptr event){
  if(level >= m_level){
    m_fileStream<<m_logFormater.format(event);
  }
}
bool FileLogAppender::reopen(){
  if(m_fileStream){
    m_fileStream.close();
  }
  m_fileStream.open(m_fileName);
  return !!m_fileStream;
}
//LogFormater{
LogFormater::LogFormater(const string & pattern):m_pattern(pattern){
  init();
}

std::string LogFormater::format(LogEvent::ptr event){
  std::stringstream ss;
  for(auto it : m_formatItems){
    it->format(ss,event);
  }
  return ss.str();
}

//%xx %xx{xxx} %%
void LogFormater::init(){
  /*
  %s%s nihao
  */
  std::vector<std::tuple<std::string,std::string,int>> vec;
  std::string pstr;      //%x
  std::string fmt;      //%{x}
  std::string str;     //xxxx%{}
  //size_t表示c中任何对象能达到的最大长度
  for(size_t i = 0;i < m_pattern.size();++i){
    if(m_pattern[i] != '%'){
      str.append(1,m_pattern[i]);
      continue;
    }
    //%%
    if(i + 1 < m_pattern.size()){
      if(m_pattern[i + 1] == '%'){
        str.append(1,'%');
        continue;
      }
    }
    //%xx{xx}
    size_t n = i + 1;
    size_t begin = i + 1;
    int status = 0;

    pstr.clear();
    fmt.clear();
    while(n < m_pattern.size()){
      if(isspace(m_pattern[n])){
        break;
      }
      if(status == 0){
        if(m_pattern[n] == '{'){
          status = 1;           //开始解析{}
          begin = n;
          pstr = m_pattern.substr(i + 1,n - i - 1;);
          ++n;
          continue;
        }
      }
      if(status == 1){
        if(m_pattern[n] == '}'){
          status = 0; //解析完毕
          fmt = m_pattern.substr(begin + 1,n - begin - 1);
          break;
        }
      }
      ++n;
      if(n == m_pattern.size()){
        if(pstr.empty()){
          pstr = m_pattern.substr(i + 1);
        }
      }
    }
    if(status == 0){
      if(!str.empty()){
        vec.push_back(std::make_tuple(str,std::string(),0));
        str.clear();
      }
      pstr = m_pattern.substr(i + 1,n - i - 1);
      vec.push_back(std::make_tuple(pstr,fmt,1));
      i = n;
    }else if(status == 1){
      std::cout<<"pattern error"<<"---"<<m_pattern.substr(i)<<std::endl;
      vec.push_back(std::make_tuple("pattern error",std::string();0));
    }
  }
  if(!str.empty()){
    vec.push_back(std::make_tuple(str,std::string(),0));
  }
  static std::map<std::string,std::function<FormaterItem::ptr(const std::string & str)> m_format_items = {
    //{str,ptr}
  #define XX(str,C) \
    {#str,[](const std::string &){return FormaterItem::ptr(new C())}}

    XX(m, MessageFormatItem),           //m:消息
    XX(p, LevelFormatItem),             //p:日志级别
    XX(r, CollapseFormatItem),            //r:累计毫秒数
    XX(c, NameFormatItem),              //c:日志名称
    XX(t, ThreadIdFormatItem),          //t:线程id
    XX(n, NewLineFormatItem),           //n:换行
    XX(d, DateTimeFormatItem),          //d:时间
    XX(f, FilenameFormatItem),          //f:文件名
    XX(l, LineFormatItem),              //l:行号
    XX(T, TabFormatItem),               //T:Tab
    XX(F, CoroutineIdFormatItem),           //F:协程id
    XX(N, ThreadNameFormatItem),        //N:线程名称
  #undef XX
  };
  for(auto& it : vec){
    if(std::get<2>(it) == 0){
      m_formatItems.push_back(new );
    }
  }


}

class MessageFormatItem :public FormaterItem{
  public:
    
    void format(std::ostream& os,LogEvent::ptr val)override{
      os<<val->m_contents;
    }
};
class LevelFormatItem :public FormaterItem{

};
class NewLineFormatItem :public FormaterItem{

};
class CollapseFormatItem :public FormaterItem{

};
class NameFormatItem :public FormaterItem{

};
class ThreadNameFormatItem :public FormaterItem{

};
class ThreadIdFormatItem :public FormaterItem{

};
class DateTimeFormatItem :public FormaterItem{

};
class FilenameFormatItem :public FormaterItem{

};
class LineFormatItem :public FormaterItem{

};
class TabFormatItem :public FormaterItem{

};
class CoroutineIdFormatItem :public FormaterItem{

};






//}
}
