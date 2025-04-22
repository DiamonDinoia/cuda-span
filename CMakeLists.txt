cmake_minimum_required(VERSION 3.8)
project(span LANGUAGES CXX)

# Optional testing
option(DD_SPAN_ENABLE_TESTING "Enable tests for dd::span" OFF)

add_library(span INTERFACE
        test/span_tests.cpp)

# Declare a proper header set
target_sources(span INTERFACE
        FILE_SET HEADERS
        BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
        FILES ${CMAKE_CURRENT_SOURCE_DIR}/include/dd/span.hpp
)

# Set include directories for consumers
target_include_directories(span INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# Require at least C++11
target_compile_features(span INTERFACE cxx_std_11)

# Testing
if(DD_SPAN_ENABLE_TESTING)
    enable_testing()
    add_subdirectory(test)
endif()
