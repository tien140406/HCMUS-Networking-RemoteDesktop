cmake_minimum_required(VERSION 3.16)

project(NetworkingApp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ----------------------------
# Find OpenCV (MSYS2 installed)
# ----------------------------
find_package(OpenCV REQUIRED)

# ----------------------------
# Manually specify curl (MSYS2 installed)
# ----------------------------
set(CURL_INCLUDE_DIR "C:/msys64/mingw64/include")
set(CURL_LIBRARY "C:/msys64/mingw64/lib/libcurl.dll.a")

include_directories(${CURL_INCLUDE_DIR})
link_directories("C:/msys64/mingw64/lib")

# ----------------------------
# Executables
# ----------------------------
add_executable(Client Client/client.cpp)
target_link_libraries(Client ws2_32)

add_executable(Server Server/server.cpp)
target_link_libraries(Server ws2_32)

add_executable(ImapTest ImapTest/main.cpp)
target_link_libraries(ImapTest
    ${CURL_LIBRARY}
    ${OpenCV_LIBS}
    ws2_32
    wldap32
)
