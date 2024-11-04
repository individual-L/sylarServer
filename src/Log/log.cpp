#include"log.hpp"
#include"config.hpp"

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
  XX(debug,DEBUG)
  XX(info,INFO)
  XX(warn,WARN)
  XX(error,ERROR)
  XX(fatal,FATAL)
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
class LoggerNameItem :public LogFormater::FormaterItem{
  public:
    LoggerNameItem(const std::string & str = ""):m_str(str){};
    void format(std::ostream& os,LogEvent::ptr val)override{
      os<<val->getLogger()->getName();
    }
  private:
    std::string m_str = "";
};

//classLogEvent
LogEvent::LogEvent(std::shared_ptr<Logger> logger,LogLevel::Level level
,const std::filesystem::path file,int32_t line,uint32_t threadId
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
  ,m_level(LogLevel::Level::INFO){
  //时间，日志级别,线程号,协程号,文件路径,行号,消息,回车
  m_logformater.reset(new gaiya::LogFormater(
    "%d{%Y-%m-%d %H:%M:%S}%T%p%T%t%T%F%T%f%T%l%T%m%n"));
}

void Logger::clearAppenders(){
  MutexType::Lock lock(m_mutex);
  m_appenders.clear();
}

void Logger::log (LogLevel::Level level,LogEvent::ptr event){
  if(level >= m_level){
    MutexType::Lock lock(m_mutex);
    if(!m_appenders.empty()){
      for(auto& it : m_appenders){
        it->log(level,event);
      }
    }else{
      m_root->log(level,event);
    }

  }
}
void Logger::addAppenders(const LogAppender::ptr appender){
  MutexType::Lock lock(m_mutex);
  if(!appender->getLogFormater()){
    MutexType::Lock alock(appender->m_mutex);
    lOG_INFO_ROOT() << "add formater";
    appender->m_logFormater = m_logformater;
  }

  m_appenders.push_back(appender);
}
void Logger::delAppenders(LogAppender::ptr appender){
  MutexType::Lock lock(m_mutex);
  for(auto it = m_appenders.begin();it != m_appenders.end();it++){
    if(*it == appender){
      m_appenders.erase(it);
      break;
    }
  }
}

void Logger::setFormater(const std::string pattern){
  //此函数并没有修改formater成员变量，故不需要加锁
  gaiya::LogFormater::ptr fmt(new gaiya::LogFormater(pattern));
  //日志格式有问题，使用默认日志格式
  if(!(fmt->isError())){
    setFormater(fmt);
  }else{
    LOG_ERROR(LOG_ROOT()) << "invaild error,formater pattern is invaild";
    throw std::invalid_argument("formater pattern is invaild");
  }
}

void Logger::setFormater(const LogFormater::ptr fmt){
  MutexType::Lock lock(m_mutex);
  m_logformater = fmt;
  for(auto& it :m_appenders){
    //每个日志输出目的地可能被多个线程共享和修改，因此需要保护其内部状态的一致性。这里的锁是针对每个目的地对象的，与Logger对象的锁是独立的。
    MutexType::Lock alock(it->m_mutex);
    if(!(it->m_hasLogFormater)){
      m_logformater = fmt;
    }
  }
}

void LogAppender::setLogformater(const LogFormater::ptr formater){
  MutexType::Lock lock(m_mutex);
  m_logFormater = formater;
  if(m_logFormater){
    m_hasLogFormater = true;
  }else{
    m_hasLogFormater = false;
  }
};
LogFormater::ptr LogAppender::getLogFormater(){
  MutexType::Lock lock(m_mutex);
  return m_logFormater;
};


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
  //防止两个日志输出到一起
  MutexType::Lock lock(m_mutex);
  if(level >= m_level){
    std::cout<<m_logFormater->format(event);
  }
}
FileLogAppender::FileLogAppender(const std::string& filename)
:m_fileName(filename){
  std::filesystem::path p = std::filesystem::current_path().parent_path();
  m_fileName = p.string() + "/loglist/"+ m_fileName;
  reopen();
}

void FileLogAppender::log(LogLevel::Level level,LogEvent::ptr event){
  if(level >= m_level){
    uint64_t now = event->getTime();
    if(now >= (m_lastTime + 3)){
      reopen();
      m_lastTime = now;
    }
    //防止两个日志输出到一起
    MutexType::Lock lock(m_mutex);
    m_logFormater->format(m_fileStream,event);
  }
}

bool FileLogAppender::reopen(){
  MutexType::Lock lock(m_mutex);
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
std::ostream& LogFormater::format(std::ostream& ofs,LogEvent::ptr event){
  for(auto it : m_formatItems){
    it->format(ofs,event);
  }
  return ofs;
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
      LOG_INFO(LOG_ROOT()) <<"pattern error"<<"---"<<m_pattern.substr(i)<<std::endl;
      m_error = true;
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
    XX(ln,LoggerNameItem)
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
  init();
}
void LoggerManager::init(){
  m_root.reset(new gaiya::Logger);
  LogAppender::ptr stdappender(new gaiya::StdLogAppender);
  stdappender->setLogformater(m_root->m_logformater);
  m_root->addAppenders(stdappender);
  m_loggers[m_root->getName()] = m_root;
}

Logger::ptr LoggerManager::getLogger(const std::string& name) {
  MutexType::Lock lock(m_mutex);
  auto it = m_loggers.find(name);

  if(it != m_loggers.end()){
    return it->second;
  }else{
    //创建新的logger
    Logger::ptr logger(new Logger(name));
    logger->m_root = m_root;
    m_loggers[name] = logger;
    return logger;
  }
}

void LoggerManager::delLogger(const std::string name){
  MutexType::Lock lock(m_mutex);
  auto it = m_loggers.find(name);
  if(it != m_loggers.end()){
    m_loggers.erase(it);
  }else{
    LOG_INFO(LOG_ROOT()) << "LoggerManager::delLogger fail, logger(" <<name <<") is not exist";
  }
}

class LogAppenderConfig{
  public:
    LogLevel::Level m_level = LogLevel::Level::UNKNOW;
    std::string m_path;
    std::string m_format;
    //1 = file,2 = std
    int m_type = 0;
    bool operator == (const LogAppenderConfig& right) const {
      if(m_level == right.m_level 
        && m_path == right.m_path
        && m_type == right.m_type
        && m_format == right.m_format){
          return true;
      }
      return false;
    }
};
class LoggerConfig{
  public:
    std::string m_name;
    std::vector<LogAppenderConfig> m_appenders;
    LogLevel::Level m_level = LogLevel::Level::UNKNOW;
    std::string m_logformat;
    bool operator == (const LoggerConfig& right) const {
      if(m_level == right.m_level 
        && m_name == right.m_name
        && m_appenders == right.m_appenders
        && m_logformat == right.m_logformat){
          return true;
      }
      return false;
    }
    //用于set的find函数进行比较
    bool operator < (const LoggerConfig& right) const {
      return m_name < right.m_name;
    }
};

template<>
class lexicalCast<std::string,LogAppenderConfig>{
  public:
    LogAppenderConfig operator() (const std::string& str){
      LogAppenderConfig res;
      YAML::Node node = YAML::Load(str);
      
      if(node["type"].IsDefined()){
        std::string type = node["type"].as<std::string>();
        if(type == "StdLogAppender"){
          res.m_type = 2;
          res.m_path = std::string();
        }else if(type == "FileLogAppender"){
          res.m_type = 1;
          if(node["path"].IsDefined()){
            res.m_path = node["path"].as<std::string>();
          }else{
            LOG_ERROR(LOG_ROOT()) << "lexicalCast<std::string,LogAppenderConfig>" << "node.path is invaild" << node;
            throw std::logic_error("node.path is invaild");
          }
        }else{ 
          LOG_ERROR(LOG_ROOT()) << "lexicalCast<std::string,LogAppenderConfig>" << "node.type is invaild,node[\"type\"] = " <<node["type"];
          throw std::logic_error("node.type is invaild");
        }
      }

      if(node["level"].IsDefined()){
        res.m_level = LogLevel::toLevel(node["level"].as<std::string>());
      }else{
        LOG_ERROR(LOG_ROOT()) << "lexicalCast<std::string,LogAppenderConfig>" << "node.level is invaild,node[\"level\"] = " <<node["level"];
        throw std::logic_error("node.level is invaild");
      }

      if(node["format"].IsDefined()){
        if(!(node["format"].IsNull())){
          res.m_format = node["format"].as<std::string>();
        }
      }else{
        LOG_ERROR(LOG_ROOT()) << "lexicalCast<std::string,LogAppenderConfig>" << "node.format is invaild,node[\"format\"] = " <<node["format"];
        throw std::logic_error("node.format is invaild");    
      }

      return res;
    }
};

template<>
class lexicalCast<LogAppenderConfig,std::string>{
  public:
    std::string operator()(const LogAppenderConfig& val){
      std::stringstream ss;
      YAML::Node node(YAML::NodeType::Map);
      node["level"] = LogLevel::toString(val.m_level);
      if(val.m_type == 1){
        node["path"] = val.m_path;
        node["type"] = "FileLogAppender";
      }else if(val.m_type == 2){
        node["type"] = "StdLogAppender";
        node["path"] = NULL;
      }
      node["format"] = val.m_format;

      ss << node;
      return ss.str();
    }
};

template<>
class lexicalCast<std::string,LoggerConfig>{
  public:
    LoggerConfig operator()(const std::string& str){
      LoggerConfig res;
      YAML::Node node = YAML::Load(str);
      if(node["level"].IsDefined()){
        res.m_level = LogLevel::toLevel(node["level"].as<std::string>());
      }else{
        LOG_ERROR(LOG_ROOT()) << "lexicalCast<std::string,LoggerConfig>" << "node.level is invaild,node[\"level\"] = " <<node["level"];
        throw std::logic_error("node.level is invaild");
      }

      if(node["name"].IsDefined()){
        res.m_name = node["name"].as<std::string>();
      }else{
        LOG_ERROR(LOG_ROOT()) << "lexicalCast<std::string,LoggerConfig>" << "node.name is invaild,node[\"name\"] = " <<node["name"];
        throw std::logic_error("node.name is invaild");
      } 

      if(node["format"].IsDefined()){
        res.m_logformat = node["format"].as<std::string>();
      }else{
        LOG_ERROR(LOG_ROOT()) << "lexicalCast<std::string,LoggerConfig>" << "node.format is invaild,node[\"format\"] = " <<node["format"];
        throw std::logic_error("node.format is invaild");
      } 

      std::stringstream ss;
      if(node["appenders"].IsDefined()){
        for(size_t i = 0;i < node["appenders"].size();++i){
          ss << node["appenders"][i];
          // LOG_INFO(LOG_ROOT()) <<std::endl << ss.str();
          res.m_appenders.push_back(lexicalCast<std::string,LogAppenderConfig>()(ss.str()));
          ss.str("");
        }
      }else{
        LOG_ERROR(LOG_ROOT()) << "lexicalCast<std::string,LoggerConfig>" << "node.appenders is invaild,node[\"appenders\"] = " <<node["appenders"];
        throw std::logic_error("node.appenders is invaild");
      } 
      return res;
    }
};

template<>
class lexicalCast<LoggerConfig,std::string>{
  public:
    std::string operator()(const LoggerConfig& val){
      YAML::Node node(YAML::NodeType::Map);
      YAML::Node vec(YAML::NodeType::Sequence);
      std::stringstream ss;
      node["name"] = val.m_name;
      node["level"] = LogLevel::toString(val.m_level);
      node["formater"] = val.m_logformat;
      for(auto& it : val.m_appenders){
        vec.push_back(YAML::Load(lexicalCast<LogAppenderConfig,std::string>()(it)));
      }
      node["appenders"] = vec;
      ss << node;
      return ss.str();
    }
};

gaiya::ConfigVar<std::set<gaiya::LoggerConfig>>::ptr config_loggers =   gaiya::Config::lookup("logs","日志器的配置参数",std::set<gaiya::LoggerConfig>{});

// 全局静态变量在程序启动时，也就是main函数执行之前就会被初始化
class Configloggerinit{
  public:
    Configloggerinit(){
      config_loggers->addCallBackFunc([](const std::set<gaiya::LoggerConfig>& oldD
      ,const std::set<gaiya::LoggerConfig>& newD)->void{
        //检测是否有新的Logger配置参数
        for(auto& it : newD){
          auto val = oldD.find(it);
          gaiya::Logger::ptr logger;
          if(val == oldD.end()){
            //新建logger
            logger = LOG_GET_LOGGER(it.m_name);
          }else{
            //修改logger
            if(!(it == (*val))){
              logger = LOG_GET_LOGGER(it.m_name);
            }else{
              continue;
            }
          }
          logger->clearAppenders();
          logger->setLevel(it.m_level);

          gaiya::LogFormater::ptr fmt(new gaiya::LogFormater(it.m_logformat));
          //日志格式有问题，则使用默认日志格式
          if(!(fmt->isError())){
            logger->setFormater(fmt);
          }

          for(auto& app : it.m_appenders){
            gaiya::LogAppender::ptr appender;
            if(app.m_type == 1){
              appender.reset( new gaiya::FileLogAppender(app.m_path));
            }else if(app.m_type == 2){
              appender.reset(new gaiya::StdLogAppender());
            }
            appender->setLevel(app.m_level);
            if(!(app.m_format.empty())){
              gaiya::LogFormater::ptr fmt(new gaiya::LogFormater(app.m_format));

              if(!(fmt->isError())){
                appender->setLogformater(fmt);
              }
            }
            logger->addAppenders(appender);
          }



        }
        //检测是否需要删除旧值
        for(auto&  it : oldD){
          auto val = newD.find(it);
          if(val == newD.end()){
            //删除旧值
            // gaiya::s_LoggersM::getInstance()->delLogger(it.m_name);
            auto logger = LOG_GET_LOGGER(it.m_name);
            logger->setLevel((LogLevel::Level)0);
            logger->clearAppenders();
          }
        }
      }
      );
    }
};
static Configloggerinit config_logger_init;

std::string LoggerManager::toYamlString() {
    MutexType::Lock lock(m_mutex);
    YAML::Node node;
    for(auto& i : m_loggers) {
        node["logs"].push_back(YAML::Load(i.second->toYamlString()));
    }
    std::stringstream ss;
    ss << node;
    return ss.str();
}
std::string StdLogAppender::toYamlString(){
  MutexType::Lock lock(m_mutex);
  YAML::Node node;
  node["type"] = "StdLogAppender";
  if(m_level != LogLevel::UNKNOW) {
      node["level"] = LogLevel::toString(m_level);
  }
  if(m_hasLogFormater && m_logFormater){
    node["format"] = m_logFormater->getPattern();
  }
  node["path"] = NULL;
  std::stringstream ss;
  ss << node;
  return ss.str();
}

std::string FileLogAppender::toYamlString(){
  MutexType::Lock lock(m_mutex);
  YAML::Node node;
  node["type"] = "FileLogAppender";
  if(m_level != LogLevel::UNKNOW) {
      node["level"] = LogLevel::toString(m_level);
  }
  if(m_hasLogFormater && m_logFormater){
    node["format"] = m_logFormater->getPattern();
  }
  node["path"] = m_fileName;
  std::stringstream ss;
  ss << node;
  return ss.str();
}

std::string Logger::toYamlString(){
  MutexType::Lock lock(m_mutex);
  YAML::Node node;
  node["name"] = m_name;
  if(m_level != LogLevel::UNKNOW) {
      node["level"] = LogLevel::toString(m_level);
  }
  if(m_logformater) {
      node["formatter"] = m_logformater->getPattern();
  }

  for(auto& i : m_appenders) {
      node["appenders"].push_back(YAML::Load(i->toYamlString()));
  }
  std::stringstream ss;
  ss << node;
  return ss.str();
}

}
