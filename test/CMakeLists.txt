find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} --std=c++11)

set(TESTS
    simple
    keywords
    array
    map
    )

foreach(TEST ${TESTS})
    message(STATUS "Test ${TEST}")
    add_executable(${TEST} ${TEST}.cpp)
    target_link_libraries(${TEST} pthread pjson ${GTEST_BOTH_LIBRARIES})
    add_test(${TEST} ${TEST})
endforeach()

