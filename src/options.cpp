#include "options.h"

Options::Options(const Options &opts) {
    network.secure = opts.network.secure;
    network.netloc = opts.network.netloc;
    network.identity = opts.network.identity;
}

bool Options::parse(int argc, const char **argv) {
    assert(argc == 2);

    auto cfg = json::parse(argv[1]);
    try {
        this->network.secure = cfg.at("secure").get<int>();;
        this->network.netloc = cfg.at("netloc").get<string>();
        this->network.identity = cfg.at("identity").get<string>();
    } catch (json::out_of_range& e) {
        cerr << e.what() << endl;
        return false;
    }

    return true;
}