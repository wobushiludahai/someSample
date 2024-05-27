cmake_minimum_required(VERSION 3.13)

project(middleware)
set(PROJECT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../ CACHE STRING "base dir")
set(TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR} CACHE STRING "base dir")

# 搜索目录下所有文件
file(GLOB SOURCE_DIR "${TEST_DIR}/*.c")

link_directories(${TEST_DIR}/output/lib)
add_executable(middleware ${SOURCE_DIR})

# 设置编译参数
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -O3 -ggdb -gdwarf -funwind-tables -rdynamic -Wno-unused-parameter -D_GNU_SOURCE")
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror -O3")

target_include_directories(middleware PRIVATE ${TEST_DIR}/test_include/sqlite)
target_include_directories(middleware PRIVATE ${TEST_DIR}/test_include/cJSON)

# 链接库
target_link_libraries(middleware sqlite3 cjson)
