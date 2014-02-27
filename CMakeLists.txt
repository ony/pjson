cmake_minimum_required(VERSION 2.8.7)
project(pjson)

# additional modules and their overrides (i.e FindGTest)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake_modules" ${CMAKE_MODULE_PATH})

set(ENABLE_TRACES FALSE CACHE BOOL "Trace to stderr all parsing steps")
set(DEVELOPMENT TRUE CACHE BOOL "Development mode (more suitable makefiles)")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Werror -Wno-unused-function -Wno-missing-field-initializers")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c1x")
if(ENABLE_TRACES)
    add_definitions(-DENABLE_TRACES)
endif()

include_directories(inc)

add_library(pjson STATIC src/pjson.c)

enable_testing()

add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND} -j4 --output-on-failure)
add_custom_target(measure)

add_subdirectory(test)
add_subdirectory(test_ansi)
add_subdirectory(test_perfomance)
