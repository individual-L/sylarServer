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
