#pragma once

#include <cstdlib>
#include <iostream>
#include <string>

inline void error(const std::string &msg) {
    std::cerr << msg << std::endl;
    std::exit(1);
}
