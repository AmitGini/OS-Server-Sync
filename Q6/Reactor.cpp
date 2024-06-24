#include "Reactor.hpp"
#include <iostream>
#include <poll.h>
#include <unistd.h>
#include <thread>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <cstring>

// Constructor
Reactor::Reactor() : running(false) {}

// Destructor
Reactor::~Reactor() {
    stop();
}

// Start the reactor in a separate thread
void* Reactor::start() {
    running = true;
    reactor_thread = std::thread(&Reactor::run, this);
    return this;
}

// Add a file descriptor and its callback to the reactor
int Reactor::addFd(int fd, reactorFunc func) {
    if (fd_map.find(fd) != fd_map.end()) return -1; // fd already exists
    struct pollfd pfd = { fd, POLLIN, 0 };
    fds.push_back(pfd);
    fd_map[fd] = func;
    return 0;
}

// Remove a file descriptor from the reactor
int Reactor::removeFd(int fd) {
    auto it = fd_map.find(fd);
    if (it == fd_map.end()) return -1; // fd doesn't exist
    fd_map.erase(it);
    fds.erase(std::remove_if(fds.begin(), fds.end(), [fd](pollfd& pfd) { return pfd.fd == fd; }), fds.end());
    close(fd);
    return 0;
}

// Stop the reactor and join(wait) the thread
int Reactor::stop() {
    running = false;
    if (reactor_thread.joinable()) reactor_thread.join();
    return 0;
}

// Reactor thread function
// Note becuase we are working on 1 Thread We dont need to lock the mutex
// Since only 1 thread is running at a time.
void Reactor::run() {
    while (running) {
        
        int ret = poll(fds.data(), fds.size(), 1000); // timeout 1s
        if (ret < 0) {
            std::cerr << "poll error: " << strerror(errno) << std::endl;
            return;
        }
        for (auto& pfd : fds) {
            if (pfd.revents & POLLIN) {
                auto it = fd_map.find(pfd.fd);
                if (it != fd_map.end()) {
                    it->second(pfd.fd); // Call the associated function
                }
            }
        }
    }
}


