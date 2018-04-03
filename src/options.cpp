#include "options.h"

bool Options::parse(int argc, const char **argv) {
    if (argc != 2) return false;
    auto cfg = json::parse(argv[1]);
    try {
        this->network.url = cfg.at("url").get<string>();
    } catch (json::out_of_range &e) {
        cerr << e.what() << endl;
        return false;
    }
    return true;
}