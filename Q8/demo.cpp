#include "Reactor.hpp"
#include "Proactor.hpp"
#include <iostream>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <fcntl.h>
#include <csignal>
#include <pthread.h>

constexpr size_t PORT = 9034;
volatile sig_atomic_t stop_server = 0; // Define stop_server

void handle_sigint(int sig) {
    stop_server = 1;
}

// Function to handle client communication
void* handleClient(int client_fd) {
    char buffer[1024];
    int bytes_received;
    

    bytes_received = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
    
    if (bytes_received <= 0) {
        if (bytes_received == 0) {
            std::cout << "Client disconnected, fd: " << client_fd << std::endl;
            removeFdFromReactor(client_fd); // Remove client from Reactor
            close(client_fd);
            return nullptr;

        } else {
            perror("recv");
            return nullptr;
        }
    }
    
    buffer[bytes_received] = '\0';
    std::string message = buffer;
    std::cout << "Received: " << buffer;

    std::string response = "Server received your message: ";
    response += message;
    send(client_fd, response.c_str(), response.size(), 0);

    
    return nullptr;
}

int main() {
    signal(SIGINT, handle_sigint); // Register signal handler

    int listener_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listener_fd == -1) {
        perror("socket");
        return 1;
    }

    int enableReuseAddr = 1;
    if (setsockopt(listener_fd, SOL_SOCKET, SO_REUSEADDR, &enableReuseAddr, sizeof(enableReuseAddr)) == -1) {
        perror("setsockopt");
        close(listener_fd);
        return 1;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(listener_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind");
        close(listener_fd);
        return 1;
    }

    if (listen(listener_fd, 10) == -1) {
        perror("listen");
        close(listener_fd);
        return 1;
    }

    std::cout << "Server is listening on port 9034" << std::endl;

    // Start the Proactor to accept connections and add them to the Reactor
    std::cout << "Starting Proactor" << std::endl;
    pthread_t proactor_thread = startProactor(listener_fd, handleClient);

    // Server loop
    while (!stop_server) {
        sleep(1);
    }

    // Clean up when done
    std::cout << "Stopping Proactor" << std::endl;
    stopProactor(proactor_thread);

    std::cout << "Stopping Reactor" << std::endl;
    stopReactor();

    close(listener_fd);

    std::cout << "Server stopped" << std::endl;

    return 0;
}
