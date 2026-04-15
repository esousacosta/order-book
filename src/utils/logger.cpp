#include "utils/logger.h"

#include <iostream>


namespace utils::logger {
    void log(const std::string &msg) {
        // For simplicity, we just print to console. In a real system, this could be more sophisticated.
        std::cout << "[LOG] " << msg << std::endl;
    }
}
