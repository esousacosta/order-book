#pragma once
#include <string>

#ifndef ACTIVE_LOG_LEVEL
#define ACTIVE_LOG_LEVEL 1
#endif

#define LOG_LEVEL_NONE 0
#define LOG_LEVEL_DEBUG 1
#define LOG_LEVEL_INFO 2

namespace utils::logger {
    void log(const std::string& msg, const std::string& logLevel = "DEBUG");
}

// Macro-based logging that compiles away if disabled
#if ACTIVE_LOG_LEVEL >= LOG_LEVEL_INFO
#define LOG_INFO(msg) utils::logger::log(msg, "INFO")
#define LOG_DEBUG(msg) ((void)0)
#elif ACTIVE_LOG_LEVEL >= LOG_LEVEL_DEBUG
#define LOG_INFO(msg) utils::logger::log(msg, "INFO")
#define LOG_DEBUG(msg) utils::logger::log(msg, "DEBUG")
#else
#define LOG_INFO(msg) ((void)0) // No-op
#define LOG_DEBUG(msg) ((void)0)
#endif
