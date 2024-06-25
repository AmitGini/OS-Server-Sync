#ifndef REACTOR_HPP
#define REACTOR_HPP

#include <vector>
#include <unordered_map>
#include <poll.h>
#include <functional>
#include <pthread.h>

typedef std::function<void(int)> reactorFunc;

extern std::vector<pollfd> fds;
extern std::unordered_map<int, reactorFunc> fd_map;
extern pthread_t reactor_thread;
extern bool running;

// initializes and starts the reactor in a separate thread.
void* startReactor();

// adds a file descriptor and its callback to the reactor.
int addFdToReactor(int fd, reactorFunc func);

// removes a file descriptor from the reactor.
int removeFdFromReactor(int fd);

// stops the reactor and joins the thread.
int stopReactor();

#endif 
