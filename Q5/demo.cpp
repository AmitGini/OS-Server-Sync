#include "Reactor.hpp"
#include <iostream>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>

#define PORT 9034

void handleClient(int client_fd) {
    char buffer[1024];

    ssize_t bytes_recv = recv(client_fd, buffer, sizeof(buffer), 0);
    if (bytes_recv > 0) {
        buffer[bytes_recv] = '\0';
        std::cout << "Received: " << buffer << std::endl;
        std::string response = "Server got your message\n";
        send(client_fd, response.c_str(), response.size(), 0);
    } else {
        std::cerr << "Client disconnected" << std::endl;
        removeFdFromReactor(client_fd);
    }
}

int getListenerSocket() {
    int listener;
    struct sockaddr_in addr;
    listener = socket(AF_INET, SOCK_STREAM, 0);
    if (listener == -1) {
        perror("socket");
        return -1;
    }

    int yes = 1;
    if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
        perror("setsockopt");
        return -1;
    }

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);

    if (bind(listener, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        perror("bind");
        return -1;
    }

    if (listen(listener, 10) == -1) {
        perror("listen");
        return -1;
    }

    return listener;
}

int main() {
    int listener = getListenerSocket();
    if (listener == -1) {
        std::cerr << "Error getting listening socket" << std::endl;
        return 1;
    }

    startReactor();
    addFdToReactor(listener, [](int fd) {
        struct sockaddr_storage remoteaddr;
        socklen_t addrlen = sizeof remoteaddr;
        int newfd = accept(fd, (struct sockaddr *)&remoteaddr, &addrlen);
        if (newfd == -1) {
            perror("accept");
            return;
        }

        char remoteIP[INET6_ADDRSTRLEN];
        void* addr;
        if (remoteaddr.ss_family == AF_INET) {
            struct sockaddr_in* s = (struct sockaddr_in*)&remoteaddr;
            addr = &(s->sin_addr);
        } else {
            struct sockaddr_in6* s = (struct sockaddr_in6*)&remoteaddr;
            addr = &(s->sin6_addr);
        }
        inet_ntop(remoteaddr.ss_family, addr, remoteIP, INET6_ADDRSTRLEN);
        printf("\npollserver: new connection from %s on socket %d\n", remoteIP, newfd);

        addFdToReactor(newfd, handleClient);
    });

    std::cout << "Reactor started. Listening for connections..." << std::endl;
    
    pause(); // Pause the main thread to keep the reactor running

    stopReactor();
    close(listener);

    return 0;
}
