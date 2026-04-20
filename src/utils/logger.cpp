#include "utils/logger.h"

#include <iostream>


namespace utils::logger {
    void log(const std::string &msg,  const std::string& logLevel) {
        // For simplicity, we just print to console. In a real system, this could be more sophisticated.
        std::cout << "[" << logLevel << "] " << msg << "\n";
    }
}
