#ifndef __LOG_H__
#define __LOG_H__
#include<string>
#include<list>
#include<sstream>
#include<fstream>
#include<memory>
#include<iostream>
#include<vector>
#include<stdint.h>
#include<stdarg.h>
#include<map>
#include<tuple>
#include<time.h>
#include<functional>
namespace gaiya{
#define LOG_LEVEL_LOGGER(level,logger) \
  if(logger->getLevel() <= level) \
    gaiya::LogEventWrap(gaiya::LogEvent::ptr(new gaiya::LogEvent( \
    logger,level,__FILE__,__LINE__ ,gaiya::GetThreadId() \
    ,gaiya::GetCoroutineId(),time(0)))).getSS()

#define LOG_DUBUG(logger) \
  LOG_LEVEL_LOGGER(gaiya::LogLevel::Level::DEBUG,logger)
#define LOG_INFO(logger) \
  LOG_LEVEL_LOGGER(gaiya::LogLevel::Level::INFO,logger)
#define LOG_WARN(logger) \
  LOG_LEVEL_LOGGER(gaiya::LogLevel::Level::WARN,logger)
#define LOG_ERROR(logger) \
  LOG_LEVEL_LOGGER(gaiya::LogLevel::Level::ERROR,logger)
#define LOG_FATAL(logger) \
  LOG_LEVEL_LOGGER(gaiya::LogLevel::Level::FATAL,logger)

#define LOG_FMT_LEVEL_LOGGER(level,logger,fmt,...) \
  if(logger->getLevel() <= level) \
    gaiya::LogEventWrap(gaiya::LogEvent::ptr(new gaiya::LogEvent( \
    logger,level,__FILE__,__LINE__ ,gaiya::GetThreadId() \
    ,gaiya::GetCoroutineId(),time(0)))).getEvent()->format(fmt,__VA_ARGS__)

#define LOG_FMT_DUBUG(logger,fmt,...) \
  LOG_FMT_LEVEL_LOGGER(gaiya::LogLevel::Level::DEBUG,logger,fmt,__VA_ARGS__)
#define LOG_FMT_INFO(logger,fmt,...) \
  LOG_FMT_LEVEL_LOGGER(gaiya::LogLevel::Level::INFO,logger,fmt,__VA_ARGS__)
#define LOG_FMT_WARN(logger,fmt,...) \
  LOG_FMT_LEVEL_LOGGER(gaiya::LogLevel::Level::WARN,logger,fmt,__VA_ARGS__)
#define LOG_FMT_ERROR(logger,fmt,...) \
  LOG_FMT_LEVEL_LOGGER(gaiya::LogLevel::Level::ERROR,logger,fmt,__VA_ARGS__)
#define LOG_FMT_FATAL(logger,fmt,...) \
  LOG_FMT_LEVEL_LOGGER(gaiya::LogLevel::Level::FATAL,logger,fmt,__VA_ARGS__)
class Logger;
class LogAppender;
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

    std::string getContents() const {return m_ss.str();};

    const char* getFile() const { return m_file;};

    int32_t getLine() const { return m_line;};

    uint32_t getElapse() const { return m_elapse;};

    uint32_t getThreadId() const { return m_threadId;};

    uint32_t getCoroutineId() const { return m_coroutineId;};

    uint64_t getTime() const { return m_time;};

    std::shared_ptr<Logger> getLogger() const {return m_logger;};

    std::stringstream& getSS()  {return m_ss;};

    const std::string& getThreadName() const { return m_threadName;};

    LogLevel::Level getLevel() const { return m_level;};

    LogEvent(std::shared_ptr<Logger> logger,LogLevel::Level level,const char * file,int32_t line,uint32_t threadId,uint32_t coroutineId,uint64_t time);

    void format(const char * fmt,...);
    void format(const char * fmt,va_list val);
  private:
    //日志器
    std::shared_ptr<Logger> m_logger;
    //日志级别
    LogLevel::Level m_level;
    //文件名
    const char * m_file = nullptr;
    //行号
    int32_t m_line = 0;
    //线程ID
    uint32_t m_threadId = 0;
    //协程ID
    uint32_t m_coroutineId = 0;
    //时间戳
    uint64_t m_time = 0;
    //程序启动到现在的毫秒数
    int32_t m_elapse = 0;
    //线程名称
    std::string m_threadName;
    //日志内容
    std::string m_contents = "";
    //日志内容流
    std::stringstream m_ss;
};
//日志格式化，将logevent转化为string
class LogFormater{
  /*
  m:消息
  p:日志级别
  r:累计毫秒数
  c:日志名称
  t:线程id
  n:换行
  d:时间
  f:文件名
  l:行号
  T:Tab
  F:协程id
  N:线程名称
  */
  public:
    typedef std::shared_ptr<LogFormater> ptr;
    //传入格式%d%T%p%T%m%n(时间，tab，消息级别，消息，换行)
    LogFormater(const std::string & pattern = "%d%T%p%T%m%n");
    //将event传给所有的formatitem，让其输出到stringstream中
    std::string format(LogEvent::ptr event);
    std::string getPattern() const {return m_pattern;};
    //日志内容(LogEvent)项格式化
    class FormaterItem{
      public:
      typedef std::shared_ptr<FormaterItem> ptr;
        virtual void format(std::ostream& os,LogEvent::ptr event) = 0;
        virtual ~FormaterItem(){};

    };
  private:
    //解析传入的格式
    void init();
  private:
    std::string m_pattern;                           //日志格式
    std::vector<FormaterItem::ptr> m_formatItems;    

};

class Logger{ 
  public:
    typedef std::shared_ptr<Logger> ptr;
    Logger(const std:: string name = "root");
    void log(LogLevel::Level level,LogEvent::ptr event);
    void addAppenders(std::shared_ptr<LogAppender> appender);
    void delAppenders(std::shared_ptr<LogAppender> appender);
    void debug(LogEvent::ptr event);
    void info(LogEvent::ptr event);
    void warn(LogEvent::ptr event);
    void error(LogEvent::ptr event);
    void fatal(LogEvent::ptr event);
    void setLevel(LogLevel::Level level){m_level = level;};
    LogLevel::Level getLevel() const {return m_level;};
    std::string getName() const {return m_name;};
  private:
    std::string m_name;
    std::list<std::shared_ptr<LogAppender>> m_appenders;
    LogLevel::Level m_level;
    LogFormater::ptr m_logformater;

};
class LogEventWrap{
public:
  LogEventWrap(LogEvent::ptr event);
  ~LogEventWrap();
  std::stringstream& getSS();
  LogEvent::ptr getEvent() const{return m_event;};
private:
  LogEvent::ptr m_event;
};
//日志输出目的地，输出器基类
class LogAppender{
  public:
    typedef std::shared_ptr<LogAppender> ptr;
    virtual ~LogAppender(){};
    //写日志，只有级别大于等于此日志器的输出级别时才输出
    virtual void log(LogLevel::Level level,LogEvent::ptr event) = 0;
    void setLogformater(LogFormater::ptr formater){m_logFormater = formater;};
    LogFormater::ptr getLogFormater() const {return m_logFormater;};
    void setLevel(LogLevel::Level level){m_level = level;};
    LogLevel::Level getLevel() const {return m_level;};
  protected:
    LogLevel::Level m_level;
    LogFormater::ptr m_logFormater;

};
//控制台输出器
class StdLogAppender :public LogAppender{
  public:
    typedef std::shared_ptr<StdLogAppender> ptr;
    void log(LogLevel::Level level,LogEvent::ptr event) override;
};
//文件输出器
class FileLogAppender :public LogAppender{
  public:
    FileLogAppender(const std::string& filename);
    typedef std::shared_ptr<FileLogAppender> ptr;
    void log(LogLevel::Level level,LogEvent::ptr event) override;
    //重新打开文件，打开成功返回true
    bool reopen();
  private:
    std::ofstream m_fileStream;
    std::string m_fileName;
    uint64_t m_lastTime = 0;
};
class LoggerManager{
  public:
    LoggerManager();
    void init();
    Logger::ptr getLogger(const std::string& name = "") const ;

    //获取主日志器
    Logger::ptr getRoot() const {return m_root;};

    // void addLogger(std::string name,Logger::ptr);
  private:
    std::map<std::string,Logger::ptr> m_loggers;
    Logger::ptr m_root; //主日志器
};




}

#endif
