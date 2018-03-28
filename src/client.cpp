#include "client.h"

Client::Client(const Options &options) : _cfg(options) {
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
    _sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (_sockfd == -1) {
        perror("cannot create socket");
        emscripten_force_exit(EXIT_FAILURE);
    }
    fcntl(_sockfd, F_SETFL, O_NONBLOCK);

    string inst = "Module['websocket']['url'] = '%s%s%s'";
    string scheme = _cfg.network.secure ? "wss://" : "ws://";
    string path = format("/agent/%s", _cfg.network.identity);
    auto set_addr = format(inst, scheme, _cfg.network.netloc, path).c_str();

    _addr.sin_family = AF_INET;
    emscripten_run_script(set_addr); // This is a weird needed override of addr.
    int res = connect(_sockfd, (struct sockaddr *) &_addr, sizeof(_addr));
    if (res == -1 && errno != EINPROGRESS) {
        perror("connect failed");
        emscripten_force_exit(EXIT_FAILURE);
    }
}

Client::~Client() {
    if (_sockfd) {
        close(_sockfd);
        _sockfd = 0;
    }
}

void Client::main() {
//    cerr << "this->_cfg.network.secure: " << to_string(this->_cfg.network.secure) << endl;
//    cerr << "this->_cfg.network.netloc: " << this->_cfg.network.netloc << endl;
//    cerr << "this->_cfg.network.identity: " << this->_cfg.network.identity << endl;

// ------------------------------------------------------------------------

//    char buffer[1024];
//    char packetLength;
//    fd_set fdr;
//    int i;
//    int res;
//
//    // make sure that sockfd is ready to read
//    FD_ZERO(&fdr);
//    FD_SET(sockfd, &fdr);
//    res = select(64, &fdr, NULL, NULL, NULL);
//    if (res == -1) {
//        perror("select failed");
//        finish(EXIT_FAILURE);
//    } else if (!FD_ISSET(sockfd, &fdr)) {
//        return;
//    }
//
//    res = recv(sockfd, buffer, 1, 0);
//    if (res == -1) {
//        if (errno == EAGAIN || errno == EWOULDBLOCK) {
//            return; //try again
//        }
//
//        perror("unexcepted end of data");
//        finish(EXIT_FAILURE);
//    }
//
//    if (res != 1) {
//        fprintf(stderr, "should read 1 byte, got: %d, sum %d so far\n", res, sum);
//        finish(EXIT_FAILURE);
//    }
//
//    packetLength = buffer[0];
//    res = recv(sockfd, buffer, packetLength, 0);
//
//    printf("got %d,%d\n", res, packetLength);
//
//    if (res != packetLength) {
//        fprintf(stderr, "lost packet data, expected: %d readed: %d", packetLength, res);
//        finish(EXIT_FAILURE);
//    }
//
//    for (i = 0; i < packetLength; ++i) {
//        if (buffer[i] != i+1) {
//            fprintf(stderr, "packet corrupted, expected: %d, actual: %d", i+1, buffer[i]);
//            finish(EXIT_FAILURE);
//        }
//
//        sum += buffer[i];
//    }
//
//    if (packetLength == buffer[0]) { // \x01\x01 - end marker
//        printf("sum: %d\n", sum);
//        finish(sum);
//    }

// ------------------------------------------------------------------------

//    static char out[1024 * 2];
//    static int pos = 0;
//    fd_set fdr;
//    fd_set fdw;
//    int res;
//
//    // make sure that server.fd is ready to read / write
//    FD_ZERO(&fdr);
//    FD_ZERO(&fdw);
//    FD_SET(server.fd, &fdr);
//    FD_SET(server.fd, &fdw);
//    res = select(64, &fdr, &fdw, nullptr, nullptr);
//    if (res == -1) {
//        perror("select failed");
//        finish(EXIT_FAILURE);
//    }
//
//    if (server.state == MSG_READ) {
//        if (!FD_ISSET(server.fd, &fdr)) {
//            return;
//        }
//
//        res = do_msg_read(server.fd, &server.msg, echo_read, 0, nullptr,
//            nullptr);
//        if (res == -1) {
//            return;
//        } else if (res == 0) {
//            perror("server closed");
//            finish(EXIT_FAILURE);
//        }
//
//        echo_read += res;
//
//        // once we've read the entire message, validate it
//        if (echo_read >= server.msg.length) {
//            assert(!strcmp(server.msg.buffer, MESSAGE));
//            finish(EXIT_SUCCESS);
//        }
//    }
//
//    if (server.state == MSG_WRITE) {
//        if (!FD_ISSET(server.fd, &fdw)) {
//            return;
//        }
//
//        res = do_msg_write(server.fd, &echo_msg, echo_wrote, 0, nullptr, 0);
//        if (res == -1) {
//            return;
//        } else if (res == 0) {
//            perror("server closed");
//            finish(EXIT_FAILURE);
//        }
//
//        echo_wrote += res;
//
//        // once we're done writing the message, read it back
//        if (echo_wrote >= echo_msg.length) {
//            server.state = MSG_READ;
//        }
//    }
}

int Client::sendall(int s, char *buf, int *len) {
    int total = 0;        // how many bytes we've sent
    int bytesleft = *len; // how many we have left to send
    int n;

    while (total < *len) {
        n = send(s, buf + total, (size_t) bytesleft, 0);
        if (n == -1) { break; }
        total += n;
        bytesleft -= n;
    }

    *len = total; // return number actually sent here

    return n == -1 ? -1 : 0; // return -1 on failure, 0 on success
}
