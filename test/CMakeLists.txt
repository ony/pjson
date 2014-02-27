find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=c++0x")

set(TESTS
    simple
    keywords
    array
    map
    str
    number
    )

foreach(TEST ${TESTS})
    message(STATUS "Test ${TEST}")
    add_executable(${TEST} ${TEST}.cpp)
    target_link_libraries(${TEST} pthread pjson ${GTEST_BOTH_LIBRARIES})
    if(DEVELOPMENT)
        add_test(${TEST} ${TEST})
    else()
        GTEST_ADD_TESTS(${TEST} "" AUTO)
    endif()
endforeach()
add_dependencies(check ${TESTS})
