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

int main() {
//    memset(&server, 0, sizeof(server_t));
//    server.state = MSG_WRITE;
//
//    // setup the message we're going to echo
//    memset(&echo_msg, 0, sizeof(msg_t));
//    echo_msg.length = strlen(MESSAGE) + 1;
//    echo_msg.buffer = (char *) malloc((size_t) echo_msg.length);
//    strncpy(echo_msg.buffer, MESSAGE, (size_t) echo_msg.length);
//
//    echo_read = 0;
//    echo_wrote = 0;
//
//    server.fd = socket(AF_INET, SOCK_STREAM, 0);
//    if (server.fd == -1) {
//        perror("cannot create socket");
//        finish(EXIT_FAILURE);
//    }
//    fcntl(server.fd, F_SETFL, O_NONBLOCK);
//
//    string inst = "Module['websocket']['url'] = '%s%s%s'";
//    string scheme = "ws://";
//    string netloc = format("%s:%d", "127.0.0.1", 8080);
//    string path = format("/agent/%s", "f64000f3-4dc9-4e22-b704-cdcf82c01038");
//    auto set_url = format(inst, scheme, netloc, path).c_str();
//
//    sockaddr_in addr{};
//    addr.sin_family = AF_INET;
//    emscripten_run_script(set_url); // This is a weird needed override of addr.
//    int res = connect(server.fd, (struct sockaddr *) &addr, sizeof(addr));
//    if (res == -1 && errno != EINPROGRESS) {
//        perror("connect failed");
//        finish(EXIT_FAILURE);
//    }

    auto *str = (char *) EM_ASM_INT({
        var string = JSON.stringify(syncaide);
        var length = lengthBytesUTF8(string) + 1;
        var pointer = _malloc(length);
        stringToUTF8(string, pointer, length + 1);
        return pointer;
    });
    auto kwargs = json::parse(str);
    free(str);

    Options options;
    printf("Address of options is %p\n", (void *)&options);
    if (!options.parse(kwargs)) {
        return EXIT_FAILURE;
    }

    Client client(options);
    printf("Address of client is %p\n", (void *)&client);
    return client.start();
}