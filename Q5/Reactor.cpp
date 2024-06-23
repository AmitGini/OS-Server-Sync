#include "Reactor.hpp"
#include <iostream>
#include <poll.h>
#include <unistd.h>
#include <thread>
#include <mutex>
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
    std::lock_guard<std::mutex> lock(mutex);
    if (fd_map.find(fd) != fd_map.end()) return -1; // fd already exists
    struct pollfd pfd = { fd, POLLIN, 0 };
    fds.push_back(pfd);
    fd_map[fd] = func;
    return 0;
}

int Reactor::removeFd(int fd) {
    std::lock_guard<std::mutex> lock(mutex);
    auto it = fd_map.find(fd);
    if (it == fd_map.end()) return -1; // fd doesn't exist
    fd_map.erase(it);
    fds.erase(std::remove_if(fds.begin(), fds.end(), [fd](pollfd& pfd) { return pfd.fd == fd; }), fds.end());
    return 0;
}

int Reactor::stop() {
    running = false;
    if (reactor_thread.joinable()) reactor_thread.join();
    return 0;
}

void Reactor::run() {
    while (running) {
        std::lock_guard<std::mutex> lock(mutex);
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

extern "C" {
    
    void* startReactor() {
        Reactor* reactor = new Reactor();
        return reactor->start();
    }

    int addFdToReactor(void* reactor, int fd, reactorFunc func) {
        Reactor* r = static_cast<Reactor*>(reactor);
        return r->addFd(fd, func);
    }

    int removeFdFromReactor(void* reactor, int fd) {
        Reactor* r = static_cast<Reactor*>(reactor);
        return r->removeFd(fd);
    }

    int stopReactor(void* reactor) {
        Reactor* r = static_cast<Reactor*>(reactor);
        int result = r->stop();
        delete r;
        return result;
    }
}
