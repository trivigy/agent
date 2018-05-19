#include "options.h"

bool Options::parse(int argc, const char **argv) {
    if (argc != 4) return false;
    verify.signature = argv[1];
    verify.digest = argv[2];

    auto cfg = json::parse(argv[3]);
    try {
        id = cfg.at("id").get<string>();
        addr = cfg.at("addr").get<string>();
        sha256 = cfg.at("sha256").get<string>();
        epoch = cfg.at("epoch").get<long>();
    } catch (exception &e) {
        cerr << e.what() << endl;
        return false;
    }
    return true;
}
