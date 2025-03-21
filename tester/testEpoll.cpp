#include <arpa/inet.h>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <netinet/in.h>
#include <string>
#include <sys/epoll.h>
#include <unistd.h>
#include<iostream>

#define MAX_EVENTS 64
#define BUF_SIZE 4096

class TcpServer {
private:
    int m_epfd; // epoll file descriptor
    int m_listen_fd; // listen file descriptor
    sockaddr_in m_server_addr;

public:
    TcpServer(const std::string& ip, int port) {
        // Create and bind the listen socket
        m_listen_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (m_listen_fd == -1) {
            perror("socket error");
            exit(1);
        }

        int opt = 1;
        setsockopt(m_listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

        memset(&m_server_addr, 0, sizeof(m_server_addr));
        m_server_addr.sin_family = AF_INET;
        inet_pton(AF_INET, ip.c_str(), &m_server_addr.sin_addr);
        m_server_addr.sin_port = htons(port);

        if (bind(m_listen_fd, (struct sockaddr*)&m_server_addr, sizeof(m_server_addr)) == -1) {
            perror("bind error");
            close(m_listen_fd);
            exit(1);
        }

        if (listen(m_listen_fd, 1024) == -1) {
            perror("listen error");
            close(m_listen_fd);
            exit(1);
        }

        // Set the listen socket to non-blocking
        int flags = fcntl(m_listen_fd, F_GETFL, 0);
        if (flags == -1) {
            perror("fcntl F_GETFL error");
            close(m_listen_fd);
            exit(1);
        }
        if (fcntl(m_listen_fd, F_SETFL, flags | O_NONBLOCK) == -1) {
            perror("fcntl F_SETFL error");
            close(m_listen_fd);
            exit(1);
        }

        // Create epoll instance
        m_epfd = epoll_create1(0);
        if (m_epfd == -1) {
            perror("epoll_create1 error");
            close(m_listen_fd);
            exit(1);
        }

        // Add listen socket to epoll
        struct epoll_event event;
        event.events = EPOLLIN | EPOLLET; // Edge-triggered mode
        event.data.fd = m_listen_fd;
        if (epoll_ctl(m_epfd, EPOLL_CTL_ADD, m_listen_fd, &event) == -1) {
            perror("epoll_ctl error");
            close(m_epfd);
            close(m_listen_fd);
            exit(1);
        }
    }

    void run() {
        struct epoll_event events[MAX_EVENTS];

        printf("Server started, listening on port %d\n", ntohs(m_server_addr.sin_port));

        while (true) {
            int nfds = epoll_wait(m_epfd, events, MAX_EVENTS, -1);
            if (nfds == -1) {
                perror("epoll_wait error");
                break;
            }

            for (int i = 0; i < nfds; ++i) {
                int fd = events[i].data.fd;

                if (fd == m_listen_fd) {
                    // Handle new connections
                    while (true) {
                        sockaddr_in client_addr;
                        socklen_t client_len = sizeof(client_addr);
                        int client_fd = accept(m_listen_fd, (sockaddr*)&client_addr, &client_len);
                        std::cout<<client_fd<<std::endl;
                        if (client_fd == -1) {
                            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                                // No more clients
                                break;
                            } else {
                                perror("accept error");
                                break;
                            }
                        }

                        // Set client socket to non-blocking
                        int flags = fcntl(client_fd, F_GETFL, 0);
                        if (flags == -1) {
                            perror("fcntl F_GETFL error");
                            close(client_fd);
                            continue;
                        }
                        if (fcntl(client_fd, F_SETFL, flags | O_NONBLOCK) == -1) {
                            perror("fcntl F_SETFL error");
                            close(client_fd);
                            continue;
                        }

                        // Add client socket to epoll
                        struct epoll_event event;
                        event.events = EPOLLIN | EPOLLET;
                        event.data.fd = client_fd;
                        if (epoll_ctl(m_epfd, EPOLL_CTL_ADD, client_fd, &event) == -1) {
                            perror("epoll_ctl error");
                            close(client_fd);
                        } else {
                            printf("New connection from %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
                        }
                    }
                } else {
                    // Handle client data
                    char buffer[BUF_SIZE];
                    while (true) {
                        ssize_t len = recv(fd, buffer, sizeof(buffer), 0);
                        if (len > 0) {
                            // Echo the data back to the client
                            send(fd, buffer, len, 0);
                            printf("Received %ld bytes from client %d\n", len, fd);
                        } else if (len == 0) {
                            // Client closed connection
                            printf("Client %d disconnected\n", fd);
                            close(fd);
                            epoll_ctl(m_epfd, EPOLL_CTL_DEL, fd, nullptr);
                            break;
                        } else if (errno == EAGAIN || errno == EWOULDBLOCK) {
                            // No more data, stop reading
                            break;
                        } else {
                            perror("recv error");
                            close(fd);
                            epoll_ctl(m_epfd, EPOLL_CTL_DEL, fd, nullptr);
                            break;
                        }
                    }
                }
            }
        }

        close(m_epfd);
        close(m_listen_fd);
    }
};

int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("Usage: %s <ip> <port>\n", argv[0]);
        return 1;
    }

    std::string ip = argv[1];
    int port = std::stoi(argv[2]);

    TcpServer server(ip, port);
    server.run();

    return 0;
}
