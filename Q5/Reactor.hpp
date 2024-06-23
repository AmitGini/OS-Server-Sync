#ifndef REACTOR_HPP
#define REACTOR_HPP
#include <vector>
#include <unordered_map>
#include <poll.h>
#include <thread>
#include <mutex>
#include <functional>

typedef std::function<void(int)> reactorFunc;

class Reactor {
public:
    Reactor();
    ~Reactor();
    
    // initializes and starts the reactor in a separate thread.
    void* start();
    // adds a file descriptor and its callback to the reactor.
    int addFd(int fd, reactorFunc func);  
    // removes a file descriptor from the reactor.
    int removeFd(int fd);
    // stops the reactor and joins the thread.
    int stop();

private:
    void run();

    std::vector<pollfd> fds;
    std::unordered_map<int, reactorFunc> fd_map;
    std::thread reactor_thread;
    std::mutex mutex;
    bool running;
};

extern "C" {
    typedef void* reactor;
    void* startReactor();
    int addFdToReactor(void* reactor, int fd, reactorFunc func);
    int removeFdFromReactor(void* reactor, int fd);
    int stopReactor(void* reactor);
}

#endif 
