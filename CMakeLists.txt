cmake_minimum_required(VERSION 3.12.0)
project(EDAProjectPartOne VERSION 0.2.0)

include(CTest)

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
if (NOT MSVC)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wno-unused-function -Wno-unused-parameter")
endif()

file(GLOB_RECURSE SOURCES RELATIVE ${CMAKE_SOURCE_DIR} "src/*.c")
add_executable(EDAProjectPartOne ${SOURCES})

enable_testing()

add_executable(scoped_heap_test "test/scoped_heap_test.c" "src/scoped_heap.c")
add_test(NAME scoped_heap_test COMMAND scoped_heap_test)

add_executable(sort_measuring_test "test/sort_measuring_test.c" "src/scoped_heap.c")

# Heap-Sort Test Coverage
add_test(NAME sort_measuring_test_1e3 COMMAND sort_measuring_test 1000)
add_test(NAME sort_measuring_test_1e5 COMMAND sort_measuring_test 100000)
add_test(NAME sort_measuring_test_1e7 COMMAND sort_measuring_test 10000000)
add_test(NAME sort_measuring_test_1e9 COMMAND sort_measuring_test 1000000000)

set(HASH_PROBING_METHOD_LINEAR 1)
set(HASH_PROBING_METHOD_QUADRATIC 2)
set(HASH_PROBING_METHOD_DOUBLE_HASHING 3)

add_executable(hash_table_measuring_test "test/hash_table_measuring_test.c" "thirdy-party/mtwister/mtwister.c")

set_target_properties(EDAProjectPartOne
					  hash_table_measuring_test
					  sort_measuring_test
					  scoped_heap_test PROPERTIES
	C_STANDARD 11
	C_STANDARD_REQUIRED ON
	C_EXTENSIONS OFF
)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

#add_custom_command(TARGET hash_table_measuring_test
#                   POST_BUILD
#                   COMMAND ctest -j1 -V -C $<CONFIGURATION> -T test --output-on-failure)
