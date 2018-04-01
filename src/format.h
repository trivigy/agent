#ifndef AGENT_FORMAT_H
#define AGENT_FORMAT_H

#include <cstdio>
#include <iostream>
#include <string>
#include <vector>

template<typename Type>
Type parse(Type value) noexcept {
    return value;
}

template<typename Type>
const Type *parse(const std::basic_string<Type> &value) noexcept {
    return value.c_str();
}


template<typename... Args>
std::string format(const std::string &format, Args ... args) {
    int size = snprintf(nullptr, 0, format.c_str(), parse(args)...) + 1;
    std::vector<char> buf((size_t) size);
    snprintf(&buf[0], (size_t) size, format.c_str(), parse(args)...);
    return std::string(buf.begin(), buf.end() - 1);
}

#endif //AGENT_FORMAT_H
