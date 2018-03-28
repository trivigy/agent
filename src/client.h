#ifndef AGENT_CLIENT_H
#define AGENT_CLIENT_H

#include "options.h"

#include <iostream>
#include <cstdlib>
#include <emscripten.h>

using namespace std;

class Client {
public:
    explicit Client(const Options &options);

    Options &cfg();

    void main();

private:
    Options _cfg;
};


#endif //AGENT_CLIENT_H
