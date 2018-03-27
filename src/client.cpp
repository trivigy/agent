#include "client.h"

Client::Client(Options &options) : _cfg(&options) {
    printf("Address of _cfg is %p\n", (void *)_cfg);
}

Options *Client::cfg() {
    return _cfg;
}

int Client::start() {
    printf("Address of this is %p\n", (void *)this);
    printf("Address of this->_cfg is %p\n", (void *)(this->_cfg));

    emscripten_set_main_loop_arg([](void *self) {
        printf("Address of self is %p\n", self);

        void (Client::*func)();
        func = &Client::main;
        (((Client *)self)->*func)();

        printf("Address of (Client *)self)->_cfg is %p\n", (void *)&(((Client *)self)->_cfg));

        cout << to_string(((Client *)self)->_cfg->network.secure) << endl;
        cout << ((Client *)self)->_cfg->network.netloc << endl;
        cout << ((Client *)self)->_cfg->network.identity << endl;

    }, this, 1, 0);
    return EXIT_SUCCESS;
}

void Client::main() {
    printf("Address of this in main is %p\n", (void *)this);
    printf("Address of this->_cfg in main is %p\n", (void *)(this->_cfg));
//    cout << to_string(this->_cfg->network.secure) << endl;
//    cout << this->_cfg->network.netloc << endl;
//    cout << this->_cfg->network.identity << endl;
}
