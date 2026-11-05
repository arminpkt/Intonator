cmake_minimum_required(VERSION 3.22)
project(UnTETered LANGUAGES C CXX)
include(cmake/CPM.cmake)

option(UNTETERED_BUILD_TESTS "Build unit tests" ON)

add_subdirectory(JUCE)
add_subdirectory(UnTETered)

if (UNTETERED_BUILD_TESTS)
    CPMAddPackage("gh:catchorg/Catch2@3.16.1")
    enable_testing()
    add_subdirectory(UnTETered/tests)
endif()
