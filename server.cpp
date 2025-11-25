#include <fcntl.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h> /* See NOTES */
#include <unistd.h>

#include <iostream>

int main() {

    // socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    // make it non-blocking
    // int flags = fcntl(sockfd, F_GETFL);
    // fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);

    // bind
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(9000);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sockfd, (const sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        return 1;
    }

    // listen: mark the socket as a passive socket. The socket will be used to
    // accept incoming connection requests using accept.
    // Incoming connections will first be queued and later accepted by
    // `accept()`.
    int backlog = 10; // maximum number of pending connections
    if (listen(sockfd, backlog) < 0) {
        perror("listen");
        return 1;
    }

    // create epoll instance
    int epfd = epoll_create1(0);
    if (epfd < 0) {
        perror("epoll_create1");
        return 1;
    }

    struct epoll_event event;
    event.events = EPOLLIN; // Monitor for input events
    event.data.fd = sockfd;
    if (epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &event)) {
        perror("epoll_ctl: clientfd");
        close(sockfd);
        exit(1);
    }

    std::cout << "Server is listening on port 9000" << std::endl;

    // while loop to accept client connections
    int maxevents = 10;
    while (true) {
        struct epoll_event event;
        event.events = EPOLLIN; // Monitor for input events
        event.data.fd = sockfd;
        int n = epoll_wait(epfd, &event, maxevents, -1);
        if (n < 0) {
            perror("epoll_wait");
            continue;
        }

        for (int i = 0; i < n; i++) {
            int fd = event.data.fd;
            if (fd == sockfd) {
                // New incoming connection
                int clientfd = accept(fd, nullptr, nullptr);

                if (clientfd < 0) {
                    perror("accept");
                    break;
                }

                // add client fd to epoll
                struct epoll_event event;
                event.events = EPOLLIN; // Monitor for input events
                event.data.fd = clientfd;
                if (epoll_ctl(epfd, EPOLL_CTL_ADD, clientfd, &event)) {
                    perror("epoll_ctl: clientfd");
                    close(clientfd);
                    break;
                }

                std::cout << "Accepted a new connection, client fd: "
                          << clientfd << std::endl;

            } else if (event.events & EPOLLIN) {
                // Data available to read on fd
            }
        }
    }

    return 0;
}