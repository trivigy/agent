#ifndef AGENT_CLIENT_H
#define AGENT_CLIENT_H

#include "format.h"
#include "options.h"
#include "protos/message.pb.h"

#include <emscripten.h>
#include <emscripten/bind.h>
#include <emscripten/val.h>
#include <functional>
#include <iostream>
#include <memory>

using namespace std;
using namespace std::placeholders;
using emscripten::val;
using emscripten::class_;

class Client : public enable_shared_from_this<Client> {
    val WebSocket = val::global("WebSocket");

private:
    Options _cfg;
    val _sock;

public:
    explicit Client(Options &options);

    void start();

private:
    void onerror(val event);

    void onopen(val event);

    void onmessage(val event);

    void onclose(val event);
};

#endif //AGENT_CLIENT_H
