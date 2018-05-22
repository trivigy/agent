#include "options.h"

bool Options::parse(int argc, const char **argv) {
    if (argc != 3) return false;
    this->argv.signature = argv[1];
    this->argv.parameters = argv[2];
    auto cfg = json::parse(this->argv.parameters);
    try {
        id = cfg.at("id").get<string>();
        addr = cfg.at("addr").get<string>();
        digest = cfg.at("digest").get<string>();
        epoch = cfg.at("epoch").get<long>();
    } catch (exception &e) {
        cerr << e.what() << endl;
        return false;
    }
    return true;
}
