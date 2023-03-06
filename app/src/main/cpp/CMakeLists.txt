cmake_minimum_required(VERSION 3.22.1)

project("vpusher")

add_library(
        vpusher
        SHARED
        native-lib.cpp VideoChannel.cpp AudioChannel.cpp SafeQueue.cpp)

add_subdirectory(librtmp)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -L${CMAKE_SOURCE_DIR}/libs/${CMAKE_ANDROID_ARCH_ABI}")
include_directories(${CMAKE_SOURCE_DIR}/include)

find_library(
        log-lib
        log)

target_link_libraries(
        vpusher
        rtmp
        x264
        ${log-lib})