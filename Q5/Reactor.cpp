#include "Reactor.hpp"
#include <iostream>
#include <unistd.h>
#include <algorithm>
#include <cstring>

std::vector<pollfd> fds; // file descriptors to check for events with poll
std::unordered_map<int, reactorFunc> fd_map; // map of file descriptors to their callbacks 
pthread_t reactor_thread; // thread for the reactor
bool running = false; // flag to stop the reactor


// The reactor thread function that runs the reactor loop.
void* reactorRun(void* arg) {
    while (running) {
        int ret = poll(fds.data(), fds.size(), 1000); // timeout 1s 
        if (ret < 0) {
            std::cerr << "poll error: " << strerror(errno) << std::endl;
            return nullptr;
        }
        // check for events on file descriptors
        for (auto& pfd : fds) {
            if (pfd.revents & POLLIN) {
                auto it = fd_map.find(pfd.fd); // find the callback for the file descriptor
                if (it != fd_map.end()) { // if the callback exists, call it
                    it->second(pfd.fd); // call the callback with the file descriptor with the right function to handle the event
                }
            }
        }
    }
    return nullptr; 
}

void* startReactor() {
    running = true;
    pthread_create(&reactor_thread, nullptr, reactorRun, nullptr); // start the reactor thread
    return nullptr;
}

int addFdToReactor(int fd, reactorFunc func) { 
    if (fd_map.find(fd) != fd_map.end()) return -1; // fd already exists
    struct pollfd pfd = { fd, POLLIN, 0 };
    fds.push_back(pfd);
    fd_map[fd] = func;
    return 0;
}

int removeFdFromReactor(int fd) {
    auto it = fd_map.find(fd);
    if (it == fd_map.end()) return -1; // fd doesn't exist
    fd_map.erase(it); // remove the callback
    fds.erase(std::remove_if(fds.begin(), fds.end(), [fd](pollfd& pfd) { return pfd.fd == fd; }), fds.end()); // remove the file descriptor
    close(fd);
    return 0;
}

int stopReactor() {
    running = false;
    if (pthread_join(reactor_thread, nullptr) != 0) { // wait for the reactor thread to finish, null ptr for no return value
        return -1;
    }
    return 0;
}