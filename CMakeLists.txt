cmake_minimum_required(VERSION 3.16)

project(NetworkingApp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Paths for curl
# Find the MSYS2 installed curl
set(CURL_INCLUDE_DIR "C:/curl/include")           # Edit if needed
set(CURL_LIBRARY "C:/curl/lib/libcurl.dll.a")       # Edit if needed

include_directories(${CURL_INCLUDE_DIR})
link_libraries(${CURL_LIBRARY})

# Targets
add_executable(Client Client/client.cpp)
target_link_libraries(Client ws2_32)

add_executable(Server Server/server.cpp)
target_link_libraries(Server ws2_32)

add_executable(ImapTest ImapTest/main.cpp)
target_link_libraries(ImapTest ${CURL_LIBRARY} ws2_32 wldap32)
