#include <iostream>
#include <string>

// Test that the version macros are defined
#ifndef OPENAUTO_VERSION_STRING
#error "OPENAUTO_VERSION_STRING not defined"
#endif

#ifndef OPENAUTO_GIT_COMMIT
#error "OPENAUTO_GIT_COMMIT not defined"
#endif

#ifndef OPENAUTO_BUILD_DATE
#error "OPENAUTO_BUILD_DATE not defined"
#endif

int main() {
    std::cout << "OpenAuto Version Test" << std::endl;
    std::cout << "===================" << std::endl;
    std::cout << "Version: " << OPENAUTO_VERSION_STRING << std::endl;
    std::cout << "Git Commit: " << OPENAUTO_GIT_COMMIT << std::endl;
    std::cout << "Build Date: " << OPENAUTO_BUILD_DATE << std::endl;
    
#ifdef OPENAUTO_GIT_BRANCH
    std::cout << "Git Branch: " << OPENAUTO_GIT_BRANCH << std::endl;
#endif

#ifdef OPENAUTO_GIT_DESCRIBE
    std::cout << "Git Describe: " << OPENAUTO_GIT_DESCRIBE << std::endl;
#endif
    
    std::cout << "✅ All version information available!" << std::endl;
    return 0;
}