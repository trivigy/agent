#include <cerrno>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <cassert>

#ifdef __EMSCRIPTEN__

#include <emscripten.h>

#endif

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
#ifdef __EMSCRIPTEN__
#ifdef REPORT_RESULT
    REPORT_RESULT(result);
#endif
    emscripten_force_exit(result);
#else
    exit(result);
#endif
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

// The callbacks for the async network events have a different signature than from
// emscripten_set_main_loop (they get passed the fd of the socket triggering the event).
// In this test application we want to try and keep as much in common as the timed loop
// version but in a real application the fd can be used instead of needing to select().
void async_main_loop(int fd, void *userData) {
    printf("%s callback\n", (char *) userData);
    main_loop();
}

void error_callback(int fd, int err, const char *msg, void *userData) {
    int error;
    socklen_t len = sizeof(error);

    int ret = getsockopt(fd, SOL_SOCKET, SO_ERROR, &error, &len);
    printf("%s callback\n", (char *) userData);
    printf("error message: %s\n", msg);

    if (err == error) {
        finish(EXIT_SUCCESS);
    } else {
        finish(EXIT_FAILURE);
    }
}

int main() {
    struct sockaddr_in addr;
    int res;

    memset(&server, 0, sizeof(server_t));
    server.state = MSG_WRITE;

    // setup the message we're going to echo
    memset(&echo_msg, 0, sizeof(msg_t));
    echo_msg.length = strlen(MESSAGE) + 1;
    echo_msg.buffer = (char *) malloc((size_t) echo_msg.length);
    strncpy(echo_msg.buffer, MESSAGE, (size_t) echo_msg.length);

    echo_read = 0;
    echo_wrote = 0;

    server.fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server.fd == -1) {
        perror("cannot create socket");
        finish(EXIT_FAILURE);
    }
    fcntl(server.fd, F_SETFL, O_NONBLOCK);

    // connect the socket
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8847);
    if (inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr) != 1) {
        perror("inet_pton failed");
        finish(EXIT_FAILURE);
    }

    res = connect(server.fd, (struct sockaddr *) &addr, sizeof(addr));
    if (res == -1 && errno != EINPROGRESS) {
        perror("connect failed");
        finish(EXIT_FAILURE);
    }

#ifdef __EMSCRIPTEN__
#if TEST_ASYNC
    // The first parameter being passed is actually an arbitrary userData pointer
  // for simplicity this test just passes a basic char*
  emscripten_set_socket_error_callback("error", error_callback);
  emscripten_set_socket_open_callback("open", async_main_loop);
  emscripten_set_socket_message_callback("message", async_main_loop);
#else
    emscripten_set_main_loop(main_loop, 60, 0);
#endif
#else
    while (1) main_loop();
#endif

    return EXIT_SUCCESS;
}