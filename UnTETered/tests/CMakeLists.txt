file (GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

add_executable(Tests ${TEST_SOURCES})

target_link_libraries(Tests PRIVATE
    UnTETered
    Catch2::Catch2WithMain)

target_include_directories(Tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)

include(${Catch2_SOURCE_DIR}/extras/Catch.cmake)
catch_discover_tests(Tests)