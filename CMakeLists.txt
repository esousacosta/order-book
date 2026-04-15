cmake_minimum_required(VERSION 3.16)
project(order_book)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_SOURCE_DIR}/include)

add_executable(order_book
        src/main.cpp
        src/core/book.cpp
        src/core/order.cpp
        include/core/book.h
        include/core/order.h
        include/core/types.h
        include/engine/matching_engine.h
        src/engine/matching_engine.cpp
        include/utils/logger.h
        src/utils/logger.cpp
)