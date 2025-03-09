#pragma once

#include <iostream>
#include <string>

inline void error(const std::string &msg) {
    std::cerr << msg << std::endl;
    exit(1);
}
