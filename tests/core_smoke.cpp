#include "openlab/core/version.hpp"

#include <iostream>

int main() {
    if (openlab::core::version() != "0.1.0") {
        std::cerr << "Unexpected core version\n";
        return 1;
    }
    return 0;
}
