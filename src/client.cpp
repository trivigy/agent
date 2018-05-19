#include "client.h"

Client::Client(Options &options) :
    _cfg(options),
    _sock(WebSocket.new_(options.addr)) {}

//Client::~Client() {
//    if (_sockfd) {
//        close(_sockfd);
//        _sockfd = 0;
//    }
//}

void Client::start() {
    auto _ = [=](auto expression) {
        function<void(val)> functor = expression;
        return val(functor)["opcall"].call<val>("bind", val(functor));
    };

    _sock.set("binaryType", val("arraybuffer"));
    _sock.set("onerror", _(bind(&Client::onerror, shared_from_this(), _1)));
    _sock.set("onopen", _(bind(&Client::onopen, shared_from_this(), _1)));
    _sock.set("onmessage", _(bind(&Client::onmessage, shared_from_this(), _1)));
    _sock.set("onclose", _(bind(&Client::onclose, shared_from_this(), _1)));
}

void Client::onerror(val event) {
    cerr << "Client::error" << endl;
}

void Client::onopen(val event) {
    cerr << "Client::open" << endl;

    protos::Message msg;
    msg.set_id(_cfg.id);
    msg.set_type(protos::MessageType::LOGIN);

    string bytes = msg.SerializeAsString();
    size_t size = msg.ByteSizeLong();
    val buffer = val::global("Uint8Array").new_(size);
    for (int i = 0; i < bytes.size(); ++i) {
        buffer.call<void>("fill", (unsigned int) bytes[i], i, i + 1);
    }

    _sock.call<void>("send", buffer);
}

void Client::onmessage(val event) {
    cerr << "Client::message" << endl;

//    protos::Message msg;
//    msg.ParseFromString(event["data"].as<string>());
//
//    cerr << "addr: " << msg.addr() << endl;
//    cerr << "age: " << msg.age() << endl;
}

void Client::onclose(val event) {
    cerr << "Client::close" << endl;
}