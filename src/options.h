#ifndef AGENT_OPTIONS_H
#define AGENT_OPTIONS_H

#include <nlohmann/json.hpp>
#include <iostream>
#include <string>

using namespace std;
using nlohmann::json;

class Options {
public:
    struct {
        string url;
    } network;

    bool parse(int argc, const char **argv);
};


#endif //AGENT_OPTIONS_H
