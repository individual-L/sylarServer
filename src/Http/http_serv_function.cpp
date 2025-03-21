#include"http_serv_function.hpp"
#include"config.hpp"
namespace gaiya{

static gaiya::Logger::ptr logger = LOG_M()->getLogger("master");

static gaiya::ConfigVar<std::string>::ptr g_content = gaiya::Config::lookup("http.default_html","http default_html",std::string("200"));
static std::string s_content;
namespace{

struct __initContent{
  __initContent(){
    g_content->addCallBackFunc([](const std::string& old,const std::string& nval){
      s_content = nval;
      // LOG_INFO(logger) <<"init html22: "<<s_content;
    });
  }
};
static __initContent initContent;
}

ServCallBack::ServCallBack(TypeCallBack cb)
:ServBase("ServCallBack")
,m_cb(cb){

}

void ServCallBack::handle(gaiya::http::HttpRequest::ptr request
              , gaiya::http::HttpResponse::ptr response 
              ) {
  return m_cb(request,response);
}
ServDefault::ServDefault()
:ServBase("DefaultServ"){
    // m_content = "<html><head><title>404 Not Found"
    //     "</title></head><body><center><h1>404 Not Found</h1></center>"
    //     "<hr><center>" + m_name + "</center></body></html>";
    m_content = s_content;
    // LOG_INFO(logger) <<m_content;
}

ServDefault::ServDefault(std::string& content)
:ServBase("DefaultServ")
,m_content(content){
  
}

ServHoldCreator::ServHoldCreator(ServBase::ptr val)
:m_serv(val){

}

void ServDefault::handle(gaiya::http::HttpRequest::ptr request
      , gaiya::http::HttpResponse::ptr response
      ){
  response->setStatus(gaiya::http::HttpStatus::OK);
  response->setHeader("Content-Type", "text/html");
  response->setBody(m_content);
  // LOG_INFO(logger) <<"使用默认处理函数";
}


ServDispatch::ServDispatch()
:ServBase("ServDispatch"){
  m_default = ServCreator<ServDefault>().get();
}

void ServDispatch::handle(gaiya::http::HttpRequest::ptr request
                , gaiya::http::HttpResponse::ptr response){
  auto serv = getMatchedServBase(request->getPath());
  if(serv){
    serv->handle(request,response);
  }
}

//
void ServDispatch::addServBase(const std::string& uri, ServBase::ptr serv){
  TypeRWMutex::WriteLock lock(m_mutex);
  m_datas[uri] =  std::make_shared<ServHoldCreator>(serv);
}

void ServDispatch::addServBase(const std::string& uri, ServCallBack::TypeCallBack cb){
  TypeRWMutex::WriteLock lock(m_mutex);
  m_datas[uri] = std::make_shared<ServHoldCreator>(std::make_shared<ServCallBack>(cb));
}

void ServDispatch::addGlobServBase(const std::string& uri, ServBase::ptr serv){
  TypeRWMutex::WriteLock lock(m_mutex);
  for(auto it = m_globs.begin();
          it != m_globs.end(); ++it) {
      if(it->first == uri) {
          m_globs.erase(it);
          break;
      }
  }
  m_globs.push_back(std::make_pair(uri,std::make_shared<ServHoldCreator>(serv)));
}

void ServDispatch::addGlobServBase(const std::string& uri, ServCallBack::TypeCallBack cb){
  TypeRWMutex::WriteLock lock(m_mutex);
  for(auto it = m_globs.begin();
          it != m_globs.end(); ++it) {
      if(it->first == uri) {
          m_globs.erase(it);
          break;
      }
  }
  m_globs.push_back(std::make_pair(uri
    ,std::make_shared<ServHoldCreator>(std::make_shared<ServCallBack>(cb))));
}

void ServDispatch::delServBase(const std::string& uri){
  TypeRWMutex::WriteLock lock(m_mutex);
  m_datas.erase(uri);
}

void ServDispatch::delGlobServBase(const std::string& uri){
  TypeRWMutex::WriteLock lock(m_mutex);
  for(auto it = m_globs.begin();
          it != m_globs.end(); ++it) {
      if(it->first == uri) {
          m_globs.erase(it);
          break;
      }
  }
}

ServBase::ptr ServDispatch::getServBase(const std::string& uri){
  TypeRWMutex::ReadLock lock(m_mutex);
  auto it = m_datas.find(uri);
  return it == m_datas.end() ? nullptr : it->second->get();
}

ServBase::ptr ServDispatch::getGlobServBase(const std::string& uri){
  TypeRWMutex::ReadLock lock(m_mutex);
  for(auto it = m_globs.begin();
          it != m_globs.end(); ++it) {
      if(it->first == uri) {
        return it->second->get();
      }
  }
  return nullptr;
}

ServBase::ptr ServDispatch::getMatchedServBase(const std::string& uri){
  TypeRWMutex::ReadLock lock(m_mutex);
  auto mit = m_datas.find(uri);
  if(mit != m_datas.end()) {
      return mit->second->get();
  }
  for(auto it = m_globs.begin();
          it != m_globs.end(); ++it) {
      if(it->first == uri) {
        return it->second->get();
      }
  }
  return m_default;
}

}
