#include "Reactor.hpp"
#include <iostream>
#include <netinet/in.h> // for sockaddr_in
#include <arpa/inet.h> // for inet_ntop
#include <unistd.h> // for close
#include <netdb.h> // for gethostbyname

#define PORT 9034

// Function to handle client connections
void handleClient(int client_fd) {
    char buffer[1024];
    ssize_t bytes_recv = recv(client_fd, buffer, sizeof(buffer), 0); // receive message from client
    if (bytes_recv > 0) {
        buffer[bytes_recv] = '\0'; // null terminate the message
        std::string message(buffer); // convert the message to a string
         if (message == "exit\n" || message == "exit\r\n") {
            std::cerr << "Client on socket "<<client_fd<<" sent exit command, disconnecting" << std::endl;
            removeFdFromReactor(client_fd);
            close(client_fd);
            return;
        } else if (message == "shutdown\n" || message == "shutdown\r\n") {
            std::cerr << "Shutdown command received, stopping server" << std::endl;
            removeFdFromReactor(client_fd);
            close(client_fd);
            stopReactor();
            return;
        }

        std::cout << "Received: " << buffer << std::endl;
        std::string response = "Server got your message\n";
        send(client_fd, response.c_str(), response.size(), 0); // send response to client
    } else {
        if (bytes_recv == 0){
            std::cerr << "Client  disconnected" << std::endl;
        } else { 
            perror("recv");
        }
        removeFdFromReactor(client_fd); // remove client from reactor
        close(client_fd); // close the client socket
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
    if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) { // allow reuse of address
        perror("setsockopt");
        return -1;
    }

    addr.sin_family = AF_INET; // IPv4
    addr.sin_addr.s_addr = INADDR_ANY; // listen on all interfaces 
    addr.sin_port = htons(PORT); // network byte order

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
    int listener = getListenerSocket(); // get the listening socket
    if (listener == -1) {
        std::cerr << "Error getting listening socket" << std::endl;
        return 1;
    }

    startReactor(); // start the reactor thread
    // add the listener to the reactor with a lambda function to handle new connections
    addFdToReactor(listener, [](int fd) {
        // accept the new connection  
        struct sockaddr_storage remoteaddr;
        socklen_t addrlen = sizeof remoteaddr;
        int newfd = accept(fd, (struct sockaddr *)&remoteaddr, &addrlen);
        if (newfd == -1) {
            perror("accept");
            return;
        }

        char remoteIP[INET6_ADDRSTRLEN]; // buffer for client IP address
        void* addr; // pointer to the address
        if (remoteaddr.ss_family == AF_INET) { // IPv4
            struct sockaddr_in* s = (struct sockaddr_in*)&remoteaddr;
            addr = &(s->sin_addr);
        } else {
            struct sockaddr_in6* s = (struct sockaddr_in6*)&remoteaddr;
            addr = &(s->sin6_addr);
        }
        inet_ntop(remoteaddr.ss_family, addr, remoteIP, INET6_ADDRSTRLEN); // convert the address to a string
        printf("\npollserver: new connection from %s on socket %d\n", remoteIP, newfd); // print the client IP address

        addFdToReactor(newfd, handleClient); // add the client to the reactor
    });

    std::cout << "Reactor started. Listening for connections..." << std::endl;
    
    // Main loop to keep the program running
    while (running) {
        sleep(1); // Sleep for 1 second
    }

    stopReactor();
    close(listener);

    std::cout << "Server has been shut down." << std::endl;

    return 0;
}