cmake_minimum_required(VERSION 3.10)
project(ADS_project1)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_BUILD_TYPE Debug)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Third party
add_library(stmr STATIC stmr/stmr.c)
target_include_directories(stmr PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/stmr)

# Executable
file(GLOB SOURCES "src/*.cpp")

add_executable(main main.cpp ${SOURCES})
target_link_libraries(main PRIVATE stmr)

# Testing
enable_testing()

file(GLOB TESTS_SRC "test/*.cpp")

add_executable(tests test/tests.cpp ${SOURCES} ${TESTS_SRC})
target_link_libraries(tests PRIVATE stmr)

add_test(NAME word_count COMMAND tests word_count)
add_test(NAME stop_filter COMMAND tests stop_filter)
add_test(NAME build_and_print_index COMMAND tests build_and_print_index)
add_test(NAME save_and_read_index COMMAND tests save_and_read_index)
add_test(NAME merge_and_print_index_file COMMAND tests merge_and_print_index_file)
add_test(NAME search_engine_gen_index COMMAND tests search_engine_gen_index)