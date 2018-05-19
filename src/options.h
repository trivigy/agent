#ifndef AGENT_OPTIONS_H
#define AGENT_OPTIONS_H

#include <nlohmann/json.hpp>
#include <iostream>
#include <string>

using namespace std;
using nlohmann::json;

class Options {
public:
    string id;
    string addr;
    string digest;
    long epoch;
    string signature;

public:
    bool parse(int argc, const char **argv);
};


#endif //AGENT_OPTIONS_H
