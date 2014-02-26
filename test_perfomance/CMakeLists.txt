include(FindPkgConfig)

find_package(GTest REQUIRED)

pkg_check_modules(YAJL yajl)
if("${YAJL_VERSION}" STREQUAL "")
    message(STATUS "YAJL-2.x not found (no comparision)")
else()
    add_definitions(-DHAVE_YAJL)
    include_directories(${YAJL_INCLUDE_DIRS})
endif()

include_directories(${GTEST_INCLUDE_DIRS})
include_directories(../test)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=c++0x")

set(JSON_BIG_SAMPLE "https://raw.github.com/openwebos/luna-init/d8a5419f570726971ca4456493159cf248508b40/files/conf/locale.txt")
set(JSON_BIG_SAMPLE_FILE "${CMAKE_CURRENT_BINARY_DIR}/big_sample.json")

if(NOT EXISTS "${JSON_BIG_SAMPLE_FILE}")
    file(DOWNLOAD "${JSON_BIG_SAMPLE}" "${JSON_BIG_SAMPLE_FILE}")
endif()
add_definitions(-DJSON_BIG_SAMPLE_FILE="${JSON_BIG_SAMPLE_FILE}")

add_executable(performance performance.cpp)
target_link_libraries(performance pthread pjson ${GTEST_BOTH_LIBRARIES} ${YAJL_LDFLAGS})

add_custom_target(measure-perfomance
    COMMAND ${CMAKE_CURRENT_BINARY_DIR}/performance
    DEPENDS performance
)
add_dependencies(measure measure-perfomance)
