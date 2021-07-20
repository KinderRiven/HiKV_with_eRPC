cmake_minimum_required(VERSION 2.8)

project(hikv)

set (CMAKE_CXX_FLAGS "-O3 -std=c++11 -mrtm")

set(SRC_BASE ${PROJECT_SOURCE_DIR})

include_directories(
)

set(SRC
    ${SRC_BASE}/city.cc
    ${SRC_BASE}/hashtable.cc
    ${SRC_BASE}/hikv.cc
)

set(LIBS
    pmem
    pthread 
    tbb
)

link_directories(
)

add_library(${PROJECT_NAME} STATIC ${SRC})
target_link_libraries(${PROJECT_NAME})
