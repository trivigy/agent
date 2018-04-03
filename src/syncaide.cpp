#include "client.h"
#include "options.h"

EMSCRIPTEN_BINDINGS(Syncaide) {
    class_<function<void(val)>>("VoidFunctorVal")
        .constructor<>()
        .function("opcall", &function<void(val)>::operator());
}

int main(int argc, const char **argv) {
    auto options = make_shared<Options>();
    if (!options->parse(argc, argv)) {
        emscripten_force_exit(EXIT_FAILURE);
    }
    make_shared<Client>(options)->run();
}