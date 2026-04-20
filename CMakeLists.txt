cmake_minimum_required(VERSION 3.16)
project(order_book)

include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.17.0.zip
)
FetchContent_MakeAvailable(googletest)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(ORDER_BOOK_SOURCES
        src/core/book.cpp
        src/core/order.cpp
        src/engine/matching_engine.cpp
        src/utils/logger.cpp)
set(ORDER_BOOK_HEADERS
        include/core/book.h
        include/core/order.h
        include/core/types.h
        include/engine/matching_engine.h
        include/utils/logger.h)

include_directories(${CMAKE_SOURCE_DIR}/include)

# Use the flag -DLOG_LEVEL to set the logging level at compile time
# example: cmake -B build -DLOG_LEVEL=DEBUG .. to set DEBUG level logging
set(LOG_LEVEL "INFO" CACHE STRING "Logging level (NONE, DEBUG, INFO)")
set_property(CACHE LOG_LEVEL PROPERTY STRINGS NONE DEBUG INFO)

# Convert string to number for C++ preprocessor
if(LOG_LEVEL STREQUAL "NONE")
    set(ACTIVE_LOG_LEVEL_NUM 0)
elseif(LOG_LEVEL STREQUAL "DEBUG")
    set(ACTIVE_LOG_LEVEL_NUM 1)
elseif(LOG_LEVEL STREQUAL "INFO")
    set(ACTIVE_LOG_LEVEL_NUM 2)
else()
    message(FATAL_ERROR "Invalid LOG_LEVEL: ${LOG_LEVEL}. Must be NONE, DEBUG, or INFO")
endif()

add_compile_definitions(ACTIVE_LOG_LEVEL=${ACTIVE_LOG_LEVEL_NUM})

# === Main executable ===
add_executable(order_book
        src/main.cpp
        ${ORDER_BOOK_SOURCES}
        ${ORDER_BOOK_HEADERS}
)

# === Tests ===
enable_testing()

add_executable(order_book_tests
tests/core/bookTest.cpp
#tests/core/orderTest.cpp
#tests/engine/matchingEngineTest.cpp
#tests/integration/endToEndTest.cpp
${ORDER_BOOK_SOURCES})
target_link_libraries(order_book_tests gtest gtest_main)
target_include_directories(order_book_tests PRIVATE ${CMAKE_SOURCE_DIR}/include)

add_test(NAME order_book_tests COMMAND order_book_tests)

if(MSVC)
    target_compile_options(order_book_tests PRIVATE /W4)
else()
    target_compile_options(order_book_tests PRIVATE -Wall -Wextra -Wpedantic)
endif()
