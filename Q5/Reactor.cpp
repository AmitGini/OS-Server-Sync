#include "Reactor.hpp"
#include <iostream>
#include <poll.h>
#include <unistd.h>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <cstring>

Reactor::Reactor() : running(false) {}

Reactor::~Reactor() {
    stop();
}

void* Reactor::start() {
    running = true;
    reactor_thread = std::thread(&Reactor::run, this);
    return this;
}

int Reactor::addFd(int fd, reactorFunc func) {
    if (fd_map.find(fd) != fd_map.end()) return -1; // fd already exists
    struct pollfd pfd = { fd, POLLIN, 0 };
    fds.push_back(pfd);
    fd_map[fd] = func;
    return 0;
}

int Reactor::removeFd(int fd) {
    auto it = fd_map.find(fd);
    if (it == fd_map.end()) return -1; // fd doesn't exist
    fd_map.erase(it);
    fds.erase(std::remove_if(fds.begin(), fds.end(), [fd](pollfd& pfd) { return pfd.fd == fd; }), fds.end());
    close(fd);
    return 0;
}

int Reactor::stop() {
    running = false;
    if (reactor_thread.joinable()) reactor_thread.join();
    return 0;
}

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
                    it->second(pfd.fd);
                }
            }
        }
    }
}

