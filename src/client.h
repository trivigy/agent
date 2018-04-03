#ifndef AGENT_CLIENT_H
#define AGENT_CLIENT_H

#include "format.h"
#include "options.h"
#include "protos/peer.pb.h"

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

public:
    explicit Client(const shared_ptr<Options> &opts);

    void onerror(val event);

    void onopen(val event);

    void onmessage(val event);

    void onclose(val event);

    void run();

private:
    const shared_ptr<Options> &_cfg;
    val _sock;
};

#endif //AGENT_CLIENT_H
