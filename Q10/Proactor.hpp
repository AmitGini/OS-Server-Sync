#ifndef PROACTOR_HPP
#define PROACTOR_HPP

#include <pthread.h> // for pthread_t
#include <csignal> // for sig_atomic_t

typedef void* (*proactorFunc)(int sockfd); // function pointer type for the proactor callback

pthread_t startProactor(int listenfd, proactorFunc threadFunc); 
int stopProactor(pthread_t tid);



#endif