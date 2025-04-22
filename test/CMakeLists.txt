set(CPM_DOWNLOAD_VERSION 0.40.8)
set(CATCH_DOWNLOAD_VERSION 3.8.1)

# USING CPM TO HANDLE DEPENDENCIES
if(CPM_SOURCE_CACHE)
    set(CPM_DOWNLOAD_LOCATION "${CPM_SOURCE_CACHE}/cpm/CPM_${CPM_DOWNLOAD_VERSION}.cmake")
elseif(DEFINED ENV{CPM_SOURCE_CACHE})
    set(CPM_DOWNLOAD_LOCATION "$ENV{CPM_SOURCE_CACHE}/cpm/CPM_${CPM_DOWNLOAD_VERSION}.cmake")
else()
    set(CPM_DOWNLOAD_LOCATION "${CMAKE_BINARY_DIR}/cmake/CPM_${CPM_DOWNLOAD_VERSION}.cmake")
endif()

if(NOT (EXISTS ${CPM_DOWNLOAD_LOCATION}))
    message(STATUS "Downloading CPM.cmake to ${CPM_DOWNLOAD_LOCATION}")
    file(
            DOWNLOAD
            https://github.com/cpm-cmake/CPM.cmake/releases/download/v${CPM_DOWNLOAD_VERSION}/CPM.cmake
            ${CPM_DOWNLOAD_LOCATION}
    )
endif()

include("${CPM_DOWNLOAD_LOCATION}")

# Use CPM to download Catch2
CPMAddPackage(
        NAME Catch2
        GITHUB_REPOSITORY catchorg/Catch2
        VERSION ${CATCH_DOWNLOAD_VERSION}
)

# Add test executable
add_executable(SpanTests span_tests.cpp)
target_include_directories(SpanTests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(SpanTests PRIVATE Catch2::Catch2WithMain span)

# Enable CTest and add the test
include(CTest)
enable_testing()
add_test(NAME SpanTests COMMAND SpanTests)
