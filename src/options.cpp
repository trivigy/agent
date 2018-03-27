#include "options.h"

bool Options::parse(const json &kwargs) {
    cout << kwargs.dump() << endl;

    try {
        this->network.secure = kwargs.at("secure");
        this->network.netloc = kwargs.at("netloc");
        this->network.identity = string("12345");
    }
    catch (json::out_of_range& e) {
        cerr << e.what() << endl;
        return false;
    }

    return true;
}
