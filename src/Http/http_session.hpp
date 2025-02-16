#ifndef __HTTP_SESSION_H__
#define __HTTP_SESSION_H__

#include "socket_stream.hpp"
#include "http.hpp"

namespace gaiya {
namespace http {


class HttpSession : public gaiya::SocketStream {
public:
    typedef std::shared_ptr<HttpSession> ptr;

    HttpSession(Socket::ptr sock, bool owner = true);

    HttpRequest::ptr recvRequest();

    int sendResponse(HttpResponse::ptr rsp);
};

}
}

#endif
