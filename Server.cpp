
#include "Server.h"
#include <signal.h>

#define QUEUE 3

Server::Server(int port) throw(const char *) {
    fd = socket(AF_INET, SOCK_STREAM, 0);   // socket
    if (fd < 0) {
        throw "socket failed";
    }
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(port);
    if (bind(fd, (struct sockaddr *) &server, sizeof(server)) < 0) { // bind
        close(fd);
        throw "bind failed";
    }
    if (listen(fd, QUEUE) < 0) { // listen
        close(fd);
        throw "listen failed";
    }

}

void helper(int signum) {
    signal(SIGALRM,helper);
}

void Server::start(ClientHandler &ch) throw(const char *) {
    t = new thread([&ch, this]() {
        signal(SIGALRM, helper);
        while (!to_Stop) {
            socklen_t clientSize = sizeof(client);
            alarm(5); // set alarm to 1 seconds
            int fd_Client = accept(fd, (struct sockaddr *) &client, &clientSize); //accept
            alarm(0);
            if (fd_Client < 0) {
                close(fd);
                throw "accept failed";
            }
            ch.handle(fd_Client);
            close(fd_Client);
        }
    });
}

void Server::stop() {
    to_Stop = true;
    close(fd); // close
    t->join(); // do not delete this!
}

Server::~Server() {
}

void AnomalyDetectionHandler::handle(int fd_Client) {
    DefaultIO *dio = new SocketIO(fd_Client);
    CLI cli(dio);
    cli.start(); // recv and send
    free(dio);
}
