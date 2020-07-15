cmake_minimum_required(VERSION 3.13)

project(distrkvs VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

configure_file("${PROJECT_NAME}Config.h.in" "${PROJECT_NAME}Config.h")

add_executable(${PROJECT_NAME} src/main.cc)

target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_BINARY_DIR})
target_include_directories(${PROJECT_NAME} PUBLIC ~/local/include)

find_library(ROCKSDB NAMES librocksdb.a HINTS ~/work/rocksdb/buildnowith REQUIRED)

target_link_libraries(${PROJECT_NAME} PUBLIC pthread dl ${ROCKSDB} stdc++fs)
