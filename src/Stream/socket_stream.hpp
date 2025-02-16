#ifndef __SOCKET_STREAM_H__
#define __SOCKET_STREAM_H__

#include"stream.hpp"
#include"socket.hpp"


namespace gaiya {

class SocketStream : public Stream {
public:
    typedef std::shared_ptr<SocketStream> ptr;

    SocketStream(Socket::ptr sock, bool owner = true);

    virtual ~SocketStream();

    virtual int read(void* buffer, size_t length) override;

    virtual int read(ByteArray::ptr ba, size_t length) override;

    virtual int write(const void* buffer, size_t length) override;

    virtual int write(ByteArray::ptr ba, size_t length) override;

    virtual void close() override;

    Socket::ptr getSocket() const { return m_socket;}

    bool isConnected() const;

protected:
    Socket::ptr m_socket;
    // 是否主控
    bool m_owner;
};

}

#endif
