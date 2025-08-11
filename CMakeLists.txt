cmake_minimum_required(VERSION 3.16)
project(NetworkingApp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

set(CURL_INCLUDE_DIR "C:/msys64/mingw64/include")
set(CURL_LIBRARY "C:/msys64/mingw64/lib/libcurl.dll.a")
include_directories(${CURL_INCLUDE_DIR})
link_directories("C:/msys64/mingw64/lib")

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

# ImapTest
add_executable(ImapTest imap/main.cpp ${APP_SOURCES})
target_link_libraries(ImapTest
    ${CURL_LIBRARY}
    ${OpenCV_LIBS}
    ws2_32
    wldap32
)