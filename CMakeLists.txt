cmake_minimum_required(VERSION 3.16)

project(NetworkingApp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find OpenCV (installed via MSYS2)
find_package(OpenCV REQUIRED)

# Manually configure libcurl (installed via MSYS2)
set(CURL_INCLUDE_DIR "C:/msys64/mingw64/include")
set(CURL_LIBRARY "C:/msys64/mingw64/lib/libcurl.dll.a")

include_directories(${CURL_INCLUDE_DIR})
link_directories("C:/msys64/mingw64/lib")

# === Gather source files ===
file(GLOB APP_SOURCES application/*.cpp)

# === Build main executable ===
add_executable(IMAP
    imap/main.cpp
    ${APP_SOURCES}
)

# === Link libraries ===
target_link_libraries(IMAP
    ${CURL_LIBRARY}
    ${OpenCV_LIBS}
    ws2_32
    wldap32
)
