set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --ansi")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --ansi")

foreach(TEST c cpp)
    message(STATUS "Test with ansi ${TEST}")
    add_executable(dummy-${TEST} main.${TEST})
    target_link_libraries(dummy-${TEST} pjson)
    add_dependencies(check dummy-${TEST})
endforeach()
