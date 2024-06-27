#ifndef REACTOR_HPP
#define REACTOR_HPP

#include <vector>
#include <unordered_map>
#include <poll.h>
#include <pthread.h>

typedef void* (*reactorFunc)(int sockfd);

void* startReactor();
int addFdToReactor(int fd, reactorFunc func);
int removeFdFromReactor(int fd);
int stopReactor();

#endif // REACTOR_HPP
