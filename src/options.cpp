#include "options.h"

bool Options::parse(int argc, const char **argv) {
    if (argc != 3) return false;
    signature = argv[1];
    auto cfg = json::parse(argv[2]);
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
