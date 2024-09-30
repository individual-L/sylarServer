#ifndef LOG_H
#define LOG_H
#include<string>
#include<list>
#include<stringstream>
#include<fstream>
#include<memory>
#include<iostream>
namespace sylar{
class LogLevel{
public:
  enum Level{
    UNKNOW = 0,
    DEBUG = 1,
    INFO = 2,
    WARN = 3,
    ERROR = 4,
    FATAL = 5
  };
  //将日志级别转化为文本类型
  static const char * toString(const LogLevel::Level level);
  //将文本类型转化为日志级别
  static LogLevel::Level toLevel(const std::string& str);
};
class LogEvent{
  public:
    typedef std::shared_ptr<LogEvent> ptr;
  private:
    //行号
    int32_t m_line = 0;
    //文件名
    const char * m_file = nullptr;
    //线程ID
    uint32_t m_threadId = 0;
    //协程ID
    uint32_t m_coroutineId = 0;
    //时间戳
    uint64_t m_time = 0;
    //程序启动到现在的毫秒数
    int32_t m_collapse = 0;
    //线程名称
    std::string m_threadName;
    //日志级别
    LogLevel::Level m_level;
    //日志内容
    std::string m_contents;
    //日志内容流
    std::stringstream m_ss;
    //日志器
    std::shared_ptr<Logger> m_logger;
};
//日志格式化
class LogFormater{
public:
  typedef std::shared_ptr<LogFormater> ptr;
  format(LogEvent::ptr event);
};

class Logger{ 
  public:
    typedef std::shared_ptr<Logger> ptr;
    Logger(const std:: string name = "root");
    void log (LogEvent::Level level,LogEvent::ptr event);
    void addAppenders(LogAppender::ptr appender);
    void delAppenders(LogAppender::ptr appender);
    void debug(LogEvent::ptr event);
    void info(LogEvent::ptr event);
    void warn(LogEvent::ptr event);
    void error(LogEvent::ptr event);
    void fatal(LogEvent::ptr event);
  private:
    std::string m_name;
    std::list<LogAppender::ptr> m_appenders;
    LogLevel::Level m_level;

};
//日志输出目的地，输出器基类
class LogAppender{
  public:
    typedef std::shared_ptr<LogAppender> ptr;
    virtual ~LogAppender(){};
    //写日志，只有级别大于等于此日志器的输出级别时才输出
    virtual void log(LogLevel::Level level,LogEvent::ptr event);
    void setLogformater(LogFormater::ptr formater){m_logFormater = formater};
    LogFormater::ptr getLogFormater() const {return m_logFormater};
  protected:
    LogLevel::Level m_level;
    LogFormater::ptr m_logFormater;

};
//控制台输出器
class StdLogAppender :public LogAppender{
  public:
    typedef std::shared_ptr<StdAppender> ptr;
    void log(LogLevel::Level level,LogEvent::ptr event) override;
};
//文件输出器
class FileLogAppender :public LogAppender{
  public:
    FileLogAppender(const std::string& filename){m_fileName = filename};
    typedef std::shared_ptr<StdAppender> ptr;
    void log(LogLevel::Level level,LogEvent::ptr event) override;
    //重新打开文件，打开成功返回true
    bool reopen();
  private:
    std::ofstream m_fileStream;
    std::string m_fileName;
};
}
#ifndef
