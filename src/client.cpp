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
}

Options &Client::cfg() {
    return _cfg;
}

void Client::main() {
    cerr << "this->_cfg.network.secure: " << to_string(this->_cfg.network.secure) << endl;
    cerr << "this->_cfg.network.netloc: " << this->_cfg.network.netloc << endl;
    cerr << "this->_cfg.network.identity: " << this->_cfg.network.identity << endl;
}