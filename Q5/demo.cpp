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

    reactor* r = static_cast<reactor*>(startReactor());
    addFdToReactor(r, fds[0], handleFd);

    // Simulate writing to the pipe
    write(fds[1], "Hello, Reactor!", 15);

    // Allow some time for the reactor to handle the event
    sleep(2);

    removeFdFromReactor(r, fds[0]);
    stopReactor(r);

    close(fds[0]);
    close(fds[1]);

    return 0;
}
