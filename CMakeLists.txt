cmake_minimum_required(VERSION 3.16)
project(NetworkingApp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Đặt file .exe output ngay trong thư mục build
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# OpenCV
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

# CURL
find_path(CURL_INCLUDE_DIR curl/curl.h
    PATHS ENV MSYS2_ROOT
    PATH_SUFFIXES mingw64/include
)
find_library(CURL_LIBRARY
    NAMES libcurl.dll.a
    PATHS ENV MSYS2_ROOT
    PATH_SUFFIXES mingw64/lib
)

include_directories(${CURL_INCLUDE_DIR})
link_directories(${CURL_LIBRARY_DIRS})

file(GLOB APP_SOURCES application/*.cpp)

# Client
add_executable(Client Client/client.cpp ${APP_SOURCES})
target_link_libraries(Client
    ${CURL_LIBRARY}
    ${OpenCV_LIBS}
    ws2_32
    wldap32
)

# Server
add_executable(Server Server/server.cpp ${APP_SOURCES})
target_link_libraries(Server
    ${CURL_LIBRARY}
    ${OpenCV_LIBS}
    ws2_32
    wldap32
)
