#include"log.h"
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
LogFormater::LogFormater(const string & pattern):m_pattern(pattern){}

std::string LogFormater::format(LogEvent::ptr event){
  std::stringstream ss;
  for(auto it : m_formatItems){
    it->format(ss,event);
  }
  return ss.str();
}
//%xx %xx{xxx} %%
void LogFormater::init(){
  //str,fmt,isStrng
  std::vector<std::tuple<std::string,std::string,int>> vec;
  std::string str;
  //size_t表示c中任何对象能达到的最大长度
  for(size_t i = 0;i < m_pattern.size();++i){
    if(m_pattern[i] != '%'){
      str.append(1,m_pattern[i]);
      continue;
    }
    if(i + 1 < m_pattern.size()){
      if(m_pattern[i + 1] == '%'){
        str.append(1,'%');
        continue;
      }
    }
    size_t n = i + 1;
    size_t begin = i + 1;
    int status = 0;

    while(n < m_pattern.size()){
      if(isspace(m_pattern[n])){
        break;
      }
      if(m_pattern[n] == '{'){

      }
      if(status == 1){

      }
    }
  }
}





//}
