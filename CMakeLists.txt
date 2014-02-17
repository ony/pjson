cmake_minimum_required(VERSION 2.8.8)
project(pjson)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Werror -Wno-unused-function")

include_directories(inc)

add_library(pjson STATIC src/pjson.c)

enable_testing()
add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure)
add_subdirectory(test)
