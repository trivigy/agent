#include "client.h"

//using namespace std;
//
//typedef struct {
//    char *buffer;
//    int length;
//} msg_t;
//
//int do_msg_read(int sockfd, msg_t *msg, int offset, int length,
//    struct sockaddr *addr, socklen_t *addrlen) {
//    int res;
//
//    if (!msg->length) {
//        // read the message length
//        res = recvfrom(sockfd, &msg->length, sizeof(int), 0, addr, addrlen);
//        if (res == -1) {
//            assert(errno == EAGAIN);
//            return res;
//        } else if (res == 0) {
//            return 0;
//        }
//
//        printf("do_msg_read: allocating %d bytes for message\n", msg->length);
//
//        msg->buffer = (char *) malloc((size_t) msg->length);
//    }
//
//    // read the actual message
//    int max = msg->length - offset;
//    if (length && max > length) {
//        max = length;
//    }
//    res = recvfrom(sockfd, msg->buffer + offset, (size_t) max, 0, addr,
//        addrlen);
//    if (res == -1) {
//        assert(errno == EAGAIN);
//        return res;
//    }
//
//    printf("do_msg_read: read %d bytes\n", res);
//
//    return res;
//}
//
//int do_msg_write(int sockfd, msg_t *msg, int offset, int length,
//    struct sockaddr *addr, socklen_t addrlen) {
//    int res;
//
//    // send the message length first
//    if (!offset) {
//        if (addr) {
//            res = sendto(sockfd, &msg->length, sizeof(int), 0, addr, addrlen);
//        } else {
//            res = send(sockfd, &msg->length, sizeof(int), 0);
//        }
//        if (res == -1) {
//            assert(errno == EAGAIN);
//            return res;
//        }
//        printf("do_msg_write: sending message header for %d bytes\n",
//            msg->length);
//        assert(res == sizeof(int));
//    }
//
//    // then the actual message
//    int max = msg->length - offset;
//    if (length && max > length) {
//        max = length;
//    }
//    if (addr) {
//        res = sendto(sockfd, msg->buffer + offset, (size_t) max, 0, addr,
//            addrlen);
//    } else {
//        res = send(sockfd, msg->buffer + offset, (size_t) max, 0);
//    }
//    if (res == -1) {
//        assert(errno == EAGAIN);
//        return res;
//    }
//
//    printf("do_msg_write: wrote %d bytes %d\n", res, msg->length);
//
//    return res;
//}
//
//// message to send to the server
//#ifndef MESSAGE
//#define MESSAGE "pingtothepong"
//#endif
//
//typedef enum {
//    MSG_READ,
//    MSG_WRITE
//} msg_state_t;
//
//typedef struct {
//    int fd;
//    msg_t msg;
//    msg_state_t state;
//} server_t;
//
//server_t server;
//msg_t echo_msg;
//int echo_read;
//int echo_wrote;
//
//void finish(int result) {
//    if (server.fd) {
//        close(server.fd);
//        server.fd = 0;
//    }
//    emscripten_force_exit(result);
//}

void error_cb(int fd, int err, const char *msg, void *self) {
    ((Client *) self)->error(fd, err, msg);
}

void open_cb(int fd, void *self) {
    ((Client *) self)->open(fd);
}

void connection_cb(int fd, void *self) {
    ((Client *) self)->connection(fd);
}

void message_cb(int fd, void *self) {
    ((Client *) self)->message(fd);
}

void close_cb(int fd, void *self) {
    ((Client *) self)->close(fd);
}

int main(int argc, const char **argv) {
    Options options;
    if (!options.parse(argc, argv)) {
        emscripten_force_exit(EXIT_FAILURE);
    }

    Client *client = new Client(options);
    emscripten_set_socket_error_callback(client, error_cb);
    emscripten_set_socket_open_callback(client, open_cb);
    emscripten_set_socket_connection_callback(client, connection_cb);
    emscripten_set_socket_message_callback(client, message_cb);
    emscripten_set_socket_close_callback(client, close_cb);

//    emscripten_set_main_loop_arg([](void *self) {
//        ((Client *)self)->main();
//    }, client, 1, 0);
}