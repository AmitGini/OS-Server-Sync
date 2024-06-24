#include "Reactor.hpp"
#include <iostream>
#include <unistd.h>

void handleFd(int fd) {
    char buffer[1024];
    int bytes = read(fd, buffer, sizeof(buffer));
    if (bytes > 0) {
        std::cout << "Read " << bytes << " bytes from fd " << fd << std::endl;
    }
}

int main() {
    int fds[2];
    pipe(fds);

    Reactor* r = new Reactor();
    r->start();
    r->addFd(fds[0], handleFd);

    // Simulate writing to the pipe
    ssize_t bytesWritten = write(fds[1], "Hello, Reactor!", 15);

    if (bytesWritten == -1) {
        std::cerr << "Error writing to pipe." << std::endl;
        // Handle the error here
    } else {
        r->addFd(fds[1], handleFd);
        fprintf(stderr, "Wrote %ld bytes to the pipe.\n", bytesWritten);
        // std::cout << "Wrote " << bytesWritten << " bytes to the pipe." << std::endl;
    }

    // Allow some time for the reactor to handle the event
    sleep(0.0009);

    r->removeFd(fds[0]); //also close fds[0]
    r->removeFd(fds[1]);  //also close fds[1]
    r->stop();

    close(fds[0]);
    close(fds[1]);
    delete r;
    return 0;
}
