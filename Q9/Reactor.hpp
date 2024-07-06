#ifndef REACTOR_HPP
#define REACTOR_HPP

#include <vector>
#include <unordered_map>
#include <poll.h>
#include <pthread.h> // for pthread_t

typedef void* (*reactorFunc)(int); // function pointer type for the reactor callback

// starts the reactor thread that runs the reactor loop. and returns a pointer to the reactor thread.
void* startReactor();

// adds a file descriptor and its callback to the reactor.
int addFdToReactor(int fd, reactorFunc func);

// removes a file descriptor from the reactor.
int removeFdFromReactor(int fd);

// stops the reactor and joins the thread.
int stopReactor();

#endif 