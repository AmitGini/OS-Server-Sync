#include "Proactor.hpp"
#include "Reactor.hpp"
#include <iostream>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <exception>


struct Data {
    int fd;
    proactorFunc func;
};

void* get_in_addr(struct sockaddr* sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void* acceptConnections(void* arg) {
    int newfd;
    try {
        struct Data* data = reinterpret_cast<Data*>(arg); // Cast the argument to Data*
        int listen_fd = data->fd; // Declare and initialize sockfd variable
        proactorFunc function = data->func;

        while (!stop_server) {  // Added stop_server check
            struct sockaddr_storage remoteaddr;
            socklen_t addrlen = sizeof(remoteaddr);
            newfd = accept(listen_fd, (struct sockaddr *)&remoteaddr, &addrlen);

            if (newfd == -1) {
                if (stop_server) {
                    break;
                }
                perror("accept");
                sleep(1);
                continue;
            }

            char remoteIP[INET6_ADDRSTRLEN];
            printf("\npollserver: new connection from %s on socket %d\n",
                   inet_ntop(remoteaddr.ss_family, get_in_addr((struct sockaddr*)&remoteaddr), remoteIP, INET6_ADDRSTRLEN), newfd);

            // Allocate memory for newfd to avoid segmentation fault
            int* newfd_ptr = new int(newfd);
            pthread_t client_thread;
            pthread_create(&client_thread, nullptr, reinterpret_cast<void* (*)(void*)>(recv), newfd_ptr);
            pthread_detach(client_thread); // Detach the thread to prevent resource leak
            addFdToReactor(newfd, function);
            std::cout << "Client thread created" << std::endl;
        }
    } catch (std::exception& e) {
        std::cerr << "Exception in acceptConnections: " << e.what() << std::endl;
        close(newfd);
        delete (Data*)arg;
    } catch (...) {
        close(newfd);
        delete (Data*)arg;
        std::cerr << "Unknown exception in acceptConnections" << std::endl;
    }
    return nullptr;
}

pthread_t startProactor(int listenfd, proactorFunc threadFunc) {
    pthread_t threadId;
    Data* data = new Data{listenfd, threadFunc};
    pthread_create(&threadId, nullptr, acceptConnections, data);

    // Start the Reactor
    startReactor();
    std::cout << "Starting Reactor" << std::endl;

    return threadId;
}

int stopProactor(pthread_t tid) {
    stop_server = 1;  // Added to ensure the server stops
    return pthread_cancel(tid);
}
