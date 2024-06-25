#include "Proactor.hpp"
#include <iostream>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

struct ProactorData {
    int sockfd;
    proactorFunc threadFunc;
};

void* acceptConnections(void* arg) {
    ProactorData* data = static_cast<ProactorData*>(arg);
    int listener_fd = data->sockfd;
    proactorFunc threadFunc = data->threadFunc;

    for (;;) {
        struct sockaddr_storage remoteaddr;
        socklen_t addrlen = sizeof remoteaddr;
        int newfd = accept(listener_fd, (struct sockaddr *)&remoteaddr, &addrlen);
        if (newfd == -1) {
            perror("accept");
            continue;
        }

        char remoteIP[INET6_ADDRSTRLEN];
        printf("\npollserver: new connection from %s on socket %d\n",
               inet_ntop(remoteaddr.ss_family, get_in_addr((struct sockaddr*)&remoteaddr), remoteIP, INET6_ADDRSTRLEN), newfd);

        pthread_t client_thread;
        pthread_create(&client_thread, nullptr, threadFunc, (void*)newfd);
        pthread_detach(client_thread);
    }
    delete data;
    return nullptr;
}

pthread_t startProactor(int sockfd, proactorFunc threadFunc) {
    pthread_t threadId;
    ProactorData* data = new ProactorData{sockfd, threadFunc};
    pthread_create(&threadId, nullptr, acceptConnections, data);
    return threadId;
}

int stopProactor(pthread_t tid) {
    return pthread_cancel(tid);
}
