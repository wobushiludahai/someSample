cmake_minimum_required(VERSION 3.13)

project(middleware)
set(PROJECT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../ CACHE STRING "base dir")
set(TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR} CACHE STRING "base dir")

add_subdirectory(${TEST_DIR}/gdbus)
add_subdirectory(${TEST_DIR}/rpc)

# 搜索目录下所有文件
file(GLOB SOURCE_FILE "${TEST_DIR}/*.c")
file(GLOB DB_FILE "${PROJECT_DIR}/db/*.c")
file(GLOB DBUS_FILE "${PROJECT_DIR}/dbus/*.c")
file(GLOB TEST_CASES_FILE "${TEST_DIR}/test_cases/*.c")

link_directories(${TEST_DIR}/output/lib)
link_directories(${TEST_DIR}/output/lib/x86_64-linux-gnu)
add_executable(middleware ${SOURCE_FILE} ${DB_FILE} ${DBUS_FILE} ${TEST_CASES_FILE})

# 设置编译参数
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -O3 -ggdb -gdwarf -funwind-tables -rdynamic -Wno-unused-parameter -D_GNU_SOURCE")
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror -O3")

target_include_directories(${PROJECT_NAME} PRIVATE ${TEST_DIR}/output/include)
target_include_directories(${PROJECT_NAME} PRIVATE ${TEST_DIR}/output/include/cjson)
target_include_directories(${PROJECT_NAME} PRIVATE ${TEST_DIR}/output/lib/x86_64-linux-gnu/glib-2.0/include)
target_include_directories(${PROJECT_NAME} PRIVATE ${TEST_DIR}/output/include/glib-2.0)
target_include_directories(${PROJECT_NAME} PRIVATE ${TEST_DIR}/output/include/gio-unix-2.0)
target_include_directories(${PROJECT_NAME} PRIVATE ${TEST_DIR}/output/lib/x86_64-linux-gnu/dbus-1.0/include)
target_include_directories(${PROJECT_NAME} PRIVATE ${TEST_DIR}/output/include/dbus-1.0)

target_include_directories(middleware PRIVATE ${TEST_DIR}/test_cases)
target_include_directories(middleware PRIVATE ${PROJECT_DIR}/db)
target_include_directories(middleware PRIVATE ${PROJECT_DIR}/dbus)

target_include_directories(middleware PRIVATE ${TEST_DIR})

# 链接库
target_link_libraries(middleware sqlite3 cjson gio-2.0 gobject-2.0 gmodule-2.0 gthread-2.0 rt glib-2.0 dbus-1 rpc)
