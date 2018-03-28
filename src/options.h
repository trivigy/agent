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
        bool secure;
        string netloc;
        string identity;
    } network;

    Options() = default;

    Options(const Options &opts);

    bool parse(int argc, const char **argv);
};


#endif //AGENT_OPTIONS_H
