#ifndef REACTOR_HPP
#define REACTOR_HPP

#include <vector>
#include <unordered_map>
#include <poll.h>
#include <functional> // for std::function
#include <pthread.h> // for pthread_t

typedef std::function<void(int)> reactorFunc; 

extern std::vector<pollfd> fds; // file descriptors to poll for events with poll
extern std::unordered_map<int, reactorFunc> fd_map; // map of file descriptors to their callbacks
extern pthread_t reactor_thread; // thread for the reactor
extern bool running; // flag to stop the reactor

// initializes and starts the reactor in a separate thread.
void* startReactor();

// adds a file descriptor and its callback to the reactor.
int addFdToReactor(int fd, reactorFunc func);

// removes a file descriptor from the reactor.
int removeFdFromReactor(int fd);

// stops the reactor and joins the thread.
int stopReactor();

#endif 