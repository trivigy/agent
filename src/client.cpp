#include "client.h"
#include "options.h"

Client::Client(const std::shared_ptr<Options> &opts) :
    _cfg(opts),
    _sock(WebSocket.new_(opts->network.url)) {}

//Client::~Client() {
//    if (_sockfd) {
//        close(_sockfd);
//        _sockfd = 0;
//    }
//}

void Client::onerror(val event) {
    cerr << "Client::error" << endl;
}

void Client::onopen(val event) {
    cerr << "Client::open" << endl;

    protos::Peer peer;
    peer.set_addr("localhost");
    peer.set_age(12345);

    string bytes = peer.SerializeAsString();
    size_t size = peer.ByteSizeLong();
    val buffer = val::global("Uint8Array").new_(size);
    for (int i = 0; i < bytes.size(); ++i) {
        buffer.call<void>("fill", (unsigned int) bytes[i], i, i + 1);
    }

    _sock.call<void>("send", buffer);
}

void Client::onmessage(val event) {
    cerr << "Client::message" << endl;
}

void Client::onclose(val event) {
    cerr << "Client::close" << endl;
}

void Client::run() {
    _sock.set("binaryType", val("ArrayBuffer"));
    [=](auto expression) {
        function<void(val)> functor = expression;
        auto adapter = val(functor)["opcall"].call<val>("bind", val(functor));
        _sock.set("onerror", adapter);
    }(bind(&Client::onerror, shared_from_this(), _1));

    [=](auto expression) {
        function<void(val)> functor = expression;
        auto adapter = val(functor)["opcall"].call<val>("bind", val(functor));
        _sock.set("onopen", adapter);
    }(bind(&Client::onopen, shared_from_this(), _1));

    [=](auto expression) {
        function<void(val)> functor = expression;
        auto adapter = val(functor)["opcall"].call<val>("bind", val(functor));
        _sock.set("onmessage", adapter);
    }(bind(&Client::onmessage, shared_from_this(), _1));

    [=](auto expression) {
        function<void(val)> functor = expression;
        auto adapter = val(functor)["opcall"].call<val>("bind", val(functor));
        _sock.set("onclose", adapter);
    }(bind(&Client::onclose, shared_from_this(), _1));
}