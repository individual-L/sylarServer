#include"log.h"
// #include"util.h"

namespace gaiya{

//LogLevel
const char * LogLevel::toString(const LogLevel::Level level){
  switch(level){
    #define XXX(str)  \
    case LogLevel::Level::str: \
      return #str; \
      break;
    XXX(DEBUG)
    XXX(INFO)
    XXX(WARN)
    XXX(ERROR)
    XXX(FATAL)
    #undef XXX
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

class MessageFormatItem :public LogFormater::FormaterItem{
  public:
    MessageFormatItem(const std::string &str = ""){};
    void format(std::ostream& os,LogEvent::ptr val)override{
      os<<val->getContents();
    }
};
class LevelFormatItem :public LogFormater::FormaterItem{
  public:
    LevelFormatItem(const std::string & str = ""){};
    void format(std::ostream& os,LogEvent::ptr val)override{
      os<<LogLevel::toString(val->getLevel());
    }
};
class NewLineFormatItem :public LogFormater::FormaterItem{
  public:
    NewLineFormatItem(const std::string & str = ""){};
    void format(std::ostream& os,LogEvent::ptr val)override{
      os<<std::endl;
    }
};
class ElapseFormatItem :public LogFormater::FormaterItem{
  public:
    ElapseFormatItem(const std::string & str = ""){};
    void format(std::ostream& os,LogEvent::ptr val)override{
      os<<val->getElapse();
    }
};
class NameFormatItem :public LogFormater::FormaterItem{
  public:
    NameFormatItem(const std::string & str = ""){};
    void format(std::ostream& os,LogEvent::ptr val)override{
      os<< val->getLogger()->getName();
    }
};
class ThreadNameFormatItem :public LogFormater::FormaterItem{
  public:
    ThreadNameFormatItem(const std::string & str = ""){};
    void format(std::ostream& os,LogEvent::ptr val)override{
      os<<val->getThreadName();
    }
};
class ThreadIdFormatItem :public LogFormater::FormaterItem{
  public:
    ThreadIdFormatItem(const std::string & str = ""){};
    void format(std::ostream& os,LogEvent::ptr val)override{
      os<<val->getThreadId();
    }
};
class DateTimeFormatItem :public LogFormater::FormaterItem{
  public:
    DateTimeFormatItem(const std::string & str = "%Y-%m-%d %H:%M:%S"):m_format(str){
      if(m_format.empty()){
        m_format = "%Y-%m-%d %H:%M:%S";
      }
    };
    void format(std::ostream& os,LogEvent::ptr val)override{
      struct tm tm;
      time_t ti = val->getTime();
      localtime_r(&ti,&tm);
      char buf[64];
      strftime(buf,sizeof(buf),m_format.c_str(),&tm);
      os<<buf;
    }
  private:
    std::string m_format;
};
class FilenameFormatItem :public LogFormater::FormaterItem{
  public:
    FilenameFormatItem(const std::string & str = ""){};
    void format(std::ostream& os,LogEvent::ptr val)override{
      os<<val->getFile();
    }
};
class LineFormatItem :public LogFormater::FormaterItem{
  public:
    LineFormatItem(const std::string & str = ""){};
    void format(std::ostream& os,LogEvent::ptr val)override{
      os<<val->getLine();
    }
};
class TabFormatItem :public LogFormater::FormaterItem{
  public:
    TabFormatItem(const std::string & str = ""){};
    void format(std::ostream& os,LogEvent::ptr val)override{
      os<<"\t";
    }
};
class CoroutineIdFormatItem :public LogFormater::FormaterItem{
  public:
    CoroutineIdFormatItem(const std::string & str = ""){};
    void format(std::ostream& os,LogEvent::ptr val)override{
      os<<val->getCoroutineId();
    }
};
class StringFormatItem :public LogFormater::FormaterItem{
  public:
    StringFormatItem(const std::string & str = ""):m_str(str){};
    void format(std::ostream& os,LogEvent::ptr val)override{
      os<<m_str;
    }
  private:
    std::string m_str = "";
};
//classLogEvent
LogEvent::LogEvent(std::shared_ptr<Logger> logger,LogLevel::Level level
,const char * file,int32_t line,uint32_t threadId
,uint32_t coroutineId,uint64_t time)
:m_logger(logger)
,m_level(level)
,m_file(file)
,m_line(line)
,m_threadId(threadId)
,m_coroutineId(coroutineId)
,m_time(time){
  
}
void LogEvent::format(const char * fmt,...){
  va_list vl;
  va_start(vl,fmt);
  format(fmt,vl);
  va_end(vl);
}

void LogEvent::format(const char * fmt,va_list val){
  char * buf = nullptr;
  int len = vasprintf(&buf,fmt,val);
  if(len != -1){
    m_ss<<std::string(buf,len);
    free(buf);
  }
}


//Logger
Logger::Logger(const std::string name)
  :m_name(name)
  ,m_level(LogLevel::Level::DEBUG){
  //时间，日志级别,线程号,协程号,文件路径,行号,消息,回车
  m_logformater.reset(new gaiya::LogFormater(
    "%d{%Y-%m-%d %H:%M:%S}%T%p%T%t%T%F%T%f%T%l%T%m%n"));
}
void Logger::log (LogLevel::Level level,LogEvent::ptr event){
  if(level >= m_level){
    for(auto it : m_appenders){
      it->log(level,event);
    }
  }
}
void Logger::addAppenders(LogAppender::ptr appender){
  if(!appender->getLogFormater()){
    appender->setLogformater(m_logformater);
  }
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
  log(LogLevel::Level::DEBUG,event);
}
void Logger::info(LogEvent::ptr event){
  log(LogLevel::Level::INFO,event);

}
void Logger::warn(LogEvent::ptr event){
  log(LogLevel::Level::WARN,event);

}
void Logger::error(LogEvent::ptr event){
  log(LogLevel::Level::ERROR,event);

}
void Logger::fatal(LogEvent::ptr event){
  log(LogLevel::Level::FATAL,event);

}
void StdLogAppender::log(LogLevel::Level level,LogEvent::ptr event){
  if(!m_logFormater){
    m_logFormater = LogFormater::ptr(new LogFormater());
  }
  if(level >= m_level){
    std::cout<<m_logFormater->format(event);
  }
}
FileLogAppender::FileLogAppender(const std::string& filename)
:m_fileName(filename){
  reopen();
}

void FileLogAppender::log(LogLevel::Level level,LogEvent::ptr event){
  if(level >= m_level){
    uint64_t now = event->getTime();
    if(now >= (m_lastTime + 3)){
      reopen();
      m_lastTime = now;
    }
    m_fileStream<<m_logFormater->format(event);
  }
}
bool FileLogAppender::reopen(){
  if(m_fileStream){
    m_fileStream.close();
  }
  m_fileStream.open(m_fileName,std::ios::app);
  if(!m_fileStream.is_open()){
    exit(1);
  }
  return !!m_fileStream;
}


//LogFormater{
LogFormater::LogFormater(const std::string & pattern):m_pattern(pattern){
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
      if(!status && (!isalpha(m_pattern[n]) && m_pattern[n] != '{'
              && m_pattern[n] != '}')) {
          pstr = m_pattern.substr(i + 1, n - i - 1);
          break;
      }
      if(status == 0){
        if(m_pattern[n] == '{'){
          status = 1;           //开始解析{}
          begin = n;
          pstr = m_pattern.substr(i + 1,n - i - 1);
          ++n;
          continue;
        }
      }
      if(status == 1){
        if(m_pattern[n] == '}'){
          status = 0; //解析完毕
          fmt = m_pattern.substr(begin + 1,n - begin - 1);
          n++;
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
      if(pstr.empty()){
        pstr = m_pattern.substr(i + 1,n - i - 1);
      }
      vec.push_back(std::make_tuple(pstr,fmt,1));
      i = n - 1;
    }else if(status == 1){
      std::cout<<"pattern error"<<"---"<<m_pattern.substr(i)<<std::endl;
      vec.push_back(std::make_tuple("pattern error",std::string(),0));
    }
  }
  if(!str.empty()){
    vec.push_back(std::make_tuple(str,std::string(),0));
  }
  static std::map<std::string,std::function<FormaterItem::ptr(const std::string & str)>> m_format_items = {
    //{str,ptr}
#define XX(str,C) \
    {#str,[](const std::string & ss){return FormaterItem::ptr( new C(ss));}}

    XX(m, MessageFormatItem),           //m:消息
    XX(p, LevelFormatItem),             //p:日志级别
    XX(r, ElapseFormatItem),            //r:累计毫秒数
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
      m_formatItems.push_back(FormaterItem::ptr(new StringFormatItem(std::get<0>(it))));
    }else{
      auto tar = m_format_items.find(std::get<0>(it));
      if(tar != m_format_items.end()){
        m_formatItems.push_back(tar->second(std::get<1>(it)));
      }else{
        m_formatItems.push_back(LogFormater::FormaterItem::ptr(
          new StringFormatItem("<<error format %" + std::get<0>(it) + ">>")));
      }
    }
  }
}
//class LogWrap
LogEventWrap::LogEventWrap(LogEvent::ptr event):m_event(event){}
LogEventWrap::~LogEventWrap(){
  m_event->getLogger()->log(m_event->getLevel(),m_event);
}
std::stringstream& LogEventWrap::getSS(){
  return m_event->getSS();
}
//class LoggerManager
LoggerManager::LoggerManager(){
  m_root.reset(new gaiya::Logger);
  m_root->addAppenders(LogAppender::ptr(new gaiya::StdLogAppender));
  gaiya::FileLogAppender::ptr file_appender(new gaiya::FileLogAppender("../tester/log.txt"));
  gaiya::LogFormater::ptr fmt(new gaiya::LogFormater   
    ("%d%T%p%T%t%T%F%T%f%T%l%T%m%n"));
  file_appender->setLogformater(fmt);
  file_appender->setLevel(gaiya::LogLevel::Level::ERROR);
  m_root->delAppenders(file_appender);
}
void LoggerManager::init(){

}
Logger::ptr LoggerManager::getLogger(const std::string& name) const {
  auto it = m_loggers.find(name);
  return it != m_loggers.end() ? it->second : m_root;
}



//}
}
