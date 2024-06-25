#include "Proactor.hpp"
#include "Reactor.hpp"
#include <iostream>
#include <pthread.h>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

#define PORT "9034"
pthread_mutex_t shared_mutex;
Reactor* reactor = nullptr;

int shared_sum = 0;

class Demo {
public:
    Demo() : listener_fd(-1) {
        pthread_mutex_init(&shared_mutex, nullptr);
    }

    ~Demo() {
        pthread_mutex_destroy(&shared_mutex);
    }

    void run() {
        listener_fd = get_listener_socket();
        if (listener_fd == -1) {
            std::cerr << "Error getting listening socket" << std::endl;
            exit(1);
        }
        std::cout << "Listener Created" << std::endl;

        reactor = new Reactor();
        reactor->start();
        std::cout << "Reactor Created and started" << std::endl;
        
        proactor_thread = startProactor(listener_fd, handle_client);
        std::cout << "Proactor Thread Created" << std::endl;

        reactor->addFd(listener_fd, client_reactor_func);
        std::cout << "Listener Added to Reactor" << std::endl;

        pthread_join(proactor_thread, nullptr);
        std::cout << "Proactor Thread Joined - Terminated" << std::endl;
        
        reactor->stop();
        delete reactor;
        std::cout << "Reactor Closed" << std::endl;

        close(listener_fd);
        std::cout << "Listener Closed" << std::endl;
    }

private:
    int listener_fd;
    pthread_t proactor_thread;

    static void* handle_client(int client_fd) {

        reactor->addFd(client_fd, client_reactor_func);
        return nullptr;
    }

    static void client_reactor_func(int client_fd) {
        try{
            char buf[256];
            memset(buf, 0, sizeof(buf));
            
            int nbytes = recv(client_fd, buf, sizeof(buf), 0);
            if (nbytes <= 0) {
                if (nbytes == 0) {
                    std::cout << "Client socket " << client_fd << " hung up" << std::endl;
                    throw std::runtime_error("Client socket hung up");
                } else {
                    throw std::runtime_error("Error reading from client socket");
                }
            }

            buf[nbytes] = '\0';

            if (strcmp(buf, "add") == 0) {
                char number_buf[256];
                int number_bytes = recv(client_fd, number_buf, sizeof(number_buf), 0);
                if (number_bytes > 0) {
                    number_buf[number_bytes] = '\0';
                    int number = std::stoi(number_buf);

                    pthread_mutex_lock(&shared_mutex);
                    shared_sum += number;
                    std::cout << "Current Sum: " << shared_sum << std::endl;
                    pthread_mutex_unlock(&shared_mutex);
                }else{
                    throw std::runtime_error("Error reading number from client socket");
                }
            }
        }
        catch (std::exception& e) {
            std::cerr << "Exception in client_reactor_func: " << e.what() << std::endl;
            close(client_fd);
        }
    }

    int get_listener_socket() {
        int listener;
        int yes = 1;
        struct addrinfo hints, *ai, *p;

        memset(&hints, 0, sizeof hints);
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags = AI_PASSIVE;

        int rv = getaddrinfo(nullptr, PORT, &hints, &ai);
        if (rv != 0) {
            fprintf(stderr, "server: %s\n", gai_strerror(rv));
            exit(1);
        }

        for (p = ai; p != nullptr; p = p->ai_next) {
            listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
            if (listener < 0) {
                continue;
            }

            setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

            if (bind(listener, p->ai_addr, p->ai_addrlen) < 0) {
                close(listener);
                continue;
            }
            break;
        }

        freeaddrinfo(ai);

        if (p == nullptr) {
            return -1;
        }

        if (listen(listener, 10) == -1) {
            return -1;
        }

        return listener;
    }
};

int main() {
    Demo demo;
    demo.run();

    return 0;
}
