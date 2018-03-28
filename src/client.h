#ifndef AGENT_CLIENT_H
#define AGENT_CLIENT_H

#include "format.h"
#include "options.h"

#include <emscripten.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>
#include <cstdlib>
#include <fcntl.h>
#include <cassert>

using namespace std;

class Client {
    typedef struct sockaddr_in sockaddr_in;

public:
    explicit Client(const Options &options);

    ~Client();

    void main();

    int sendall(int s, char *buf, int *len);

private:
    int _sockfd;
    Options _cfg;
    sockaddr_in _addr;
};


#endif //AGENT_CLIENT_H
