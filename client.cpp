#include <fcntl.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h> /* See NOTES */
#include <unistd.h>
#include <arpa/inet.h>

#include <iostream>

int main() {
    int clientfd = socket(AF_INET, SOCK_STREAM, 0);
    if (clientfd < 0) {
        perror("socket");
        return 1;
    }

    // connect
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(9000);
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

    if (connect(clientfd, (sockaddr*)&addr, sizeof(addr)) < 0) {
        if (errno != EINPROGRESS) {
            perror("connect"); return 1;
        }
    }

    // create epoll instance
    int epfd = epoll_create1(0);
    if (epfd < 0) {
        perror("epoll_create1");
        return 1;
    }

    struct epoll_event event;
    event.events = EPOLLIN; // Monitor for input events
    event.data.fd = clientfd;
    if (epoll_ctl(epfd, EPOLL_CTL_ADD, clientfd, &event)) {
        perror("epoll_ctl: clientfd");
        close(clientfd);
        exit(1);
    }

    // while (true) {

    // }

    return 0;
}