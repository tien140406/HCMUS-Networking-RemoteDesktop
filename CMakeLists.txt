cmake_minimum_required(VERSION 3.10)
project(ImapTest)

# C++17 tiêu chuẩn
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Tự động dò libcurl từ MSYS2
find_package(CURL REQUIRED)

# Tạo file thực thi từ main.cpp
add_executable(ImapTest ImapTest/main.cpp)

# Liên kết thư viện curl
target_link_libraries(ImapTest PRIVATE CURL::libcurl)

# Windows-specific libraries nếu cần (để dùng ShellExecute, v.v.)
if (WIN32)
    target_link_libraries(ImapTest PRIVATE ws2_32 crypt32 winmm)
endif()
