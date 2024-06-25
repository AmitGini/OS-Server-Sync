#ifndef PROACTOR_HPP
#define PROACTOR_HPP

#include <pthread.h>

typedef void* (*proactorFunc)(int sockfd);

pthread_t startProactor(int sockfd, proactorFunc threadFunc);
int stopProactor(pthread_t tid);

#endif 
