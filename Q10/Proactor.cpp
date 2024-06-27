#include "Proactor.hpp"
#include "Reactor.hpp"
#include <iostream>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <exception>
#include <fcntl.h>
#include <sys/select.h>

pthread_mutex_t proactor_mutex = PTHREAD_MUTEX_INITIALIZER;
int isServerInitStop = 0; // Initialize stop_server to 0
int self_pipe[2]; // Self-pipe for waking up from accept

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
    int newfd = -1; // Initialize newfd to an invalid value
    try {
        struct Data* data = reinterpret_cast<Data*>(arg); // Cast the argument to Data*
        int listen_fd = data->fd; // Declare and initialize sockfd variable
        proactorFunc function = data->func;

        while (!isServerInitStop) {  // stop looping if server is trying to closed
            newfd = -1;
            struct sockaddr_storage remoteaddr;
            socklen_t addrlen = sizeof(remoteaddr);

            fd_set readfds;
            FD_ZERO(&readfds);
            FD_SET(listen_fd, &readfds);
            FD_SET(self_pipe[0], &readfds);
            int maxfd = std::max(listen_fd, self_pipe[0]);

            int activity = select(maxfd + 1, &readfds, NULL, NULL, NULL);

            if (activity < 0 && errno != EINTR) {
                perror("select error");
                break;
            }

            if (FD_ISSET(self_pipe[0], &readfds)) {
                char buf[1];
                read(self_pipe[0], buf, 1);
                if (isServerInitStop) {
                    delete data;
                    std::cout << "Init Stopping Server" << std::endl;
                    break;
                }
            }

            if (FD_ISSET(listen_fd, &readfds)) {
                newfd = accept(listen_fd, (struct sockaddr *)&remoteaddr, &addrlen);

                if (isServerInitStop) {
                    if (newfd != -1) close(newfd); // Close newfd if it was opened
                    delete data;
                    std::cout << "Init Stopping Server" << std::endl;
                    break;
                }

                if (newfd == -1) {
                    perror("accept");
                    sleep(1);
                    continue; // continue to retry accept on failure
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
        }
    
    } catch (std::exception& e) {
        std::cerr << "Exception in acceptConnections: " << e.what() << std::endl;
        if (newfd != -1) close(newfd);  // Ensure newfd is closed if an exception occurs
        delete (Data*)arg;
    
    } catch (...) {
        std::cerr << "Unknown exception in acceptConnections" << std::endl;
        if (newfd != -1) close(newfd);  // Ensure newfd is closed if an unknown exception occurs
        delete (Data*)arg;
    }

    return nullptr;
}

pthread_t startProactor(int listenfd, proactorFunc threadFunc) {
    pthread_t threadId;
    Data* data = new Data{listenfd, threadFunc};
    if (pipe(self_pipe) == -1) {
        perror("pipe");
        exit(1);
    }
    pthread_create(&threadId, nullptr, acceptConnections, data);

    return threadId;
}

int stopProactor(pthread_t tid) {
    isServerInitStop = 1;  // Signal all threads to stop
    write(self_pipe[1], "x", 1); // Wake up the accept thread

    close(self_pipe[0]);
    close(self_pipe[1]);
    
    std::cout<<"Finished Stopping Reactor"<<std::endl;

    return 0;
}
