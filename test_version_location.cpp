// Test file to verify the new Version.hpp location
#include "modern/Version.hpp"
#include <iostream>

int main() {
    std::cout << "Testing Version.hpp in modern/ directory:" << std::endl;
    std::cout << "Version: " << openauto::modern::getVersionString() << std::endl;
    std::cout << "Full info: " << openauto::modern::getFullVersionInfo() << std::endl;
    return 0;
}
