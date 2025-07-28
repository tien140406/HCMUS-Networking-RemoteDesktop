cmake_minimum_required(VERSION 3.16)
project(NetworkingApp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

message(STATUS "=== Portable CURL Detection ===")

# Method 1: Try pkg-config (most reliable for MSYS2)
find_package(PkgConfig )
if(PkgConfig_FOUND)
    message(STATUS "Trying pkg-config for libcurl...")
    pkg_check_modules(CURL QUIET libcurl)
    if(CURL_FOUND)
        message(STATUS "✓ Found libcurl via pkg-config")
        message(STATUS "  Version: ${CURL_VERSION}")
        message(STATUS "  Include dirs: ${CURL_INCLUDE_DIRS}")
        message(STATUS "  Libraries: ${CURL_LIBRARIES}")
        set(CURL_METHOD "pkg-config")
    else()
        message(STATUS "✗ pkg-config couldn't find libcurl")
    endif()
endif()
find_package(PkgConfig )
if(PkgConfig_FOUND)
    message(STATUS "Trying pkg-config for libcurl...")
    pkg_check_modules(CURL QUIET libcurl)
    if(CURL_FOUND)
        message(STATUS "✓ Found libcurl via pkg-config")
        message(STATUS "  Version: ${CURL_VERSION}")
        message(STATUS "  Include dirs: ${CURL_INCLUDE_DIRS}")
        message(STATUS "  Libraries: ${CURL_LIBRARIES}")
        set(CURL_METHOD "pkg-config")
    else()
        message(STATUS "✗ pkg-config couldn't find libcurl")
    endif()
endif()

# Method 2: Manual search with portable paths
if(NOT CURL_FOUND)
    message(STATUS "Trying manual search...")
if(NOT CURL_FOUND)
    message(STATUS "Trying manual search...")
    
    # Common MSYS2/MinGW installation paths
    set(CURL_SEARCH_PATHS
        $ENV{MINGW_PREFIX}
        $ENV{MSYSTEM_PREFIX}
        /mingw64
        /mingw32
        /usr/local
        /usr
    )
    # Common MSYS2/MinGW installation paths
    set(CURL_SEARCH_PATHS
        $ENV{MINGW_PREFIX}
        $ENV{MSYSTEM_PREFIX}
        /mingw64
        /mingw32
        /usr/local
        /usr
    )
    
    # Also check common Windows locations
    if(WIN32)
        list(APPEND CURL_SEARCH_PATHS
            C:/msys64/mingw64
            D:/msys64/mingw64
            C:/mingw64
            D:/mingw64
        )
    endif()
    # Also check common Windows locations
    if(WIN32)
        list(APPEND CURL_SEARCH_PATHS
            C:/msys64/mingw64
            D:/msys64/mingw64
            C:/mingw64
            D:/mingw64
        )
    endif()
    
    find_path(CURL_INCLUDE_DIR
        NAMES curl/curl.h
        PATHS ${CURL_SEARCH_PATHS}
        PATH_SUFFIXES include
        DOC "CURL include directory"
    )
    find_path(CURL_INCLUDE_DIR
        NAMES curl/curl.h
        PATHS ${CURL_SEARCH_PATHS}
        PATH_SUFFIXES include
        DOC "CURL include directory"
    )
    
    find_library(CURL_LIBRARY
        NAMES curl libcurl
        PATHS ${CURL_SEARCH_PATHS}
        PATH_SUFFIXES lib lib64
        DOC "CURL library"
    )
    find_library(CURL_LIBRARY
        NAMES curl libcurl
        PATHS ${CURL_SEARCH_PATHS}
        PATH_SUFFIXES lib lib64
        DOC "CURL library"
    )
    
    if(CURL_INCLUDE_DIR AND CURL_LIBRARY)
        set(CURL_FOUND TRUE)
        set(CURL_INCLUDE_DIRS ${CURL_INCLUDE_DIR})
        set(CURL_LIBRARIES ${CURL_LIBRARY})
        set(CURL_METHOD "manual")
        message(STATUS "✓ Found libcurl manually")
        message(STATUS "  Include dir: ${CURL_INCLUDE_DIR}")
        message(STATUS "  Library: ${CURL_LIBRARY}")
    else()
        message(STATUS "✗ Manual search failed")
    endif()
endif()
    if(CURL_INCLUDE_DIR AND CURL_LIBRARY)
        set(CURL_FOUND TRUE)
        set(CURL_INCLUDE_DIRS ${CURL_INCLUDE_DIR})
        set(CURL_LIBRARIES ${CURL_LIBRARY})
        set(CURL_METHOD "manual")
        message(STATUS "✓ Found libcurl manually")
        message(STATUS "  Include dir: ${CURL_INCLUDE_DIR}")
        message(STATUS "  Library: ${CURL_LIBRARY}")
    else()
        message(STATUS "✗ Manual search failed")
    endif()
endif()

# Final check
if(NOT CURL_FOUND)
    message(FATAL_ERROR "
Could not find libcurl!

Installation instructions:
- MSYS2: pacman -S mingw-w64-x86_64-curl
- Ubuntu/Debian: sudo apt-get install libcurl4-openssl-dev
- CentOS/RHEL: sudo yum install libcurl-devel
- macOS: brew install curl

Make sure libcurl is installed and either:
1. pkg-config can find it (recommended)
2. It's in a standard system location
3. Set CMAKE_PREFIX_PATH to point to the installation directory
")
endif()

message(STATUS "✓ CURL detection complete using: ${CURL_METHOD}")

# # OpenCV
# if(WIN32)
#     set(OpenCV_DIR "C:/msys64/mingw64/lib/cmake/opencv4")
# endif()

find_package(OpenCV REQUIRED)
message(STATUS "✓ OpenCV found: ${OpenCV_VERSION}")

# Application source files
file(GLOB APP_SOURCES application/*.cpp)

# Build the executable
add_executable(IMAP
    imap/main.cpp
    ${APP_SOURCES}
)

# Set include directories
target_include_directories(IMAP PRIVATE
    ${SRC_DIR}
    ${CURL_INCLUDE_DIRS}
    ${OpenCV_INCLUDE_DIRS}
)

# Link libraries
target_link_libraries(IMAP
    ${CURL_LIBRARIES}
    ${OpenCV_LIBS}
    ws2_32
    wldap32
)