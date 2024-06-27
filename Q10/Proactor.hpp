#ifndef PROACTOR_HPP
#define PROACTOR_HPP

#include <pthread.h>
#include <csignal>

typedef void* (*proactorFunc)(int sockfd);

pthread_t startProactor(int listenfd, proactorFunc threadFunc);
int stopProactor(pthread_t tid);

#endif
