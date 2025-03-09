cmake_minimum_required(VERSION 3.11)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

find_library(liboqs NAMES liboqs PATHS ${CMAKE_CURRENT_SOURCE_DIR}/lib REQUIRED)

set(SRC run.c)
set(HEADER run.h)

add_library(libOQS STATIC ${SRC} ${HEADER})

target_link_libraries(libOQS PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/lib/liboqs.a)
target_include_directories(libOQS PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(libOQS PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(libOQS PUBLIC ${CMAKE_SOURCE_DIR})
