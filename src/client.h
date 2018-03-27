#ifndef AGENT_CLIENT_H
#define AGENT_CLIENT_H

#include "options.h"

#include <iostream>
#include <cstdlib>
#include <emscripten.h>

using namespace std;

class Client {
public:
    explicit Client(Options &options);

    Options *cfg();

    int start();

private:
    Options *_cfg;

    void main();
};


#endif //AGENT_CLIENT_H
