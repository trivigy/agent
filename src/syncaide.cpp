#include "format.h"
#include "client.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <cassert>

using namespace std;

typedef struct sockaddr_in sockaddr_in;

typedef struct {
    char *buffer;
    int length;
} msg_t;

int do_msg_read(int sockfd, msg_t *msg, int offset, int length,
    struct sockaddr *addr, socklen_t *addrlen) {
    int res;

    if (!msg->length) {
        // read the message length
        res = recvfrom(sockfd, &msg->length, sizeof(int), 0, addr, addrlen);
        if (res == -1) {
            assert(errno == EAGAIN);
            return res;
        } else if (res == 0) {
            return 0;
        }

        printf("do_msg_read: allocating %d bytes for message\n", msg->length);

        msg->buffer = (char *) malloc((size_t) msg->length);
    }

    // read the actual message
    int max = msg->length - offset;
    if (length && max > length) {
        max = length;
    }
    res = recvfrom(sockfd, msg->buffer + offset, (size_t) max, 0, addr,
        addrlen);
    if (res == -1) {
        assert(errno == EAGAIN);
        return res;
    }

    printf("do_msg_read: read %d bytes\n", res);

    return res;
}

int do_msg_write(int sockfd, msg_t *msg, int offset, int length,
    struct sockaddr *addr, socklen_t addrlen) {
    int res;

    // send the message length first
    if (!offset) {
        if (addr) {
            res = sendto(sockfd, &msg->length, sizeof(int), 0, addr, addrlen);
        } else {
            res = send(sockfd, &msg->length, sizeof(int), 0);
        }
        if (res == -1) {
            assert(errno == EAGAIN);
            return res;
        }
        printf("do_msg_write: sending message header for %d bytes\n",
            msg->length);
        assert(res == sizeof(int));
    }

    // then the actual message
    int max = msg->length - offset;
    if (length && max > length) {
        max = length;
    }
    if (addr) {
        res = sendto(sockfd, msg->buffer + offset, (size_t) max, 0, addr,
            addrlen);
    } else {
        res = send(sockfd, msg->buffer + offset, (size_t) max, 0);
    }
    if (res == -1) {
        assert(errno == EAGAIN);
        return res;
    }

    printf("do_msg_write: wrote %d bytes %d\n", res, msg->length);

    return res;
}

// message to send to the server
#ifndef MESSAGE
#define MESSAGE "pingtothepong"
#endif

typedef enum {
    MSG_READ,
    MSG_WRITE
} msg_state_t;

typedef struct {
    int fd;
    msg_t msg;
    msg_state_t state;
} server_t;

server_t server;
msg_t echo_msg;
int echo_read;
int echo_wrote;

void finish(int result) {
    if (server.fd) {
        close(server.fd);
        server.fd = 0;
    }
    emscripten_force_exit(result);
}

void main_loop() {
    static char out[1024 * 2];
    static int pos = 0;
    fd_set fdr;
    fd_set fdw;
    int res;

    // make sure that server.fd is ready to read / write
    FD_ZERO(&fdr);
    FD_ZERO(&fdw);
    FD_SET(server.fd, &fdr);
    FD_SET(server.fd, &fdw);
    res = select(64, &fdr, &fdw, nullptr, nullptr);
    if (res == -1) {
        perror("select failed");
        finish(EXIT_FAILURE);
    }

    if (server.state == MSG_READ) {
        if (!FD_ISSET(server.fd, &fdr)) {
            return;
        }

        res = do_msg_read(server.fd, &server.msg, echo_read, 0, nullptr,
            nullptr);
        if (res == -1) {
            return;
        } else if (res == 0) {
            perror("server closed");
            finish(EXIT_FAILURE);
        }

        echo_read += res;

        // once we've read the entire message, validate it
        if (echo_read >= server.msg.length) {
            assert(!strcmp(server.msg.buffer, MESSAGE));
            finish(EXIT_SUCCESS);
        }
    }

    if (server.state == MSG_WRITE) {
        if (!FD_ISSET(server.fd, &fdw)) {
            return;
        }

        res = do_msg_write(server.fd, &echo_msg, echo_wrote, 0, nullptr, 0);
        if (res == -1) {
            return;
        } else if (res == 0) {
            perror("server closed");
            finish(EXIT_FAILURE);
        }

        echo_wrote += res;

        // once we're done writing the message, read it back
        if (echo_wrote >= echo_msg.length) {
            server.state = MSG_READ;
        }
    }
}

int main(int argc, const char **argv) {
    Options options;
    if (!options.parse(argc, argv)) {
        emscripten_force_exit(EXIT_FAILURE);
    }

    emscripten_set_main_loop_arg([](void *self) {
        void (Client::*func)();
        func = &Client::main;
        (((Client *)self)->*func)();
    }, new Client(options), 1, 0);
}