cmake_minimum_required(VERSION 3.16)
project(NetworkingApp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# ========== Find OpenGL ==========
find_package(OpenGL REQUIRED)

# ========== Find OpenCV ==========
find_package(OpenCV REQUIRED)

# ========== Find/Fetch CURL ==========
find_package(CURL)

# if(NOT CURL_FOUND)
#     message(STATUS "Trying manual search...")
#     set(CURL_SEARCH_PATHS
#         $ENV{MINGW_PREFIX}
#         $ENV{MSYSTEM_PREFIX}
#         /mingw64
#         /mingw32
#         /usr/local
#         /usr
#     )
    
#     if(WIN32)
#         list(APPEND CURL_SEARCH_PATHS
#             C:/msys64/mingw64
#             D:/msys64/mingw64
#             C:/mingw64
#             D:/mingw64
#         )
#     endif()
    
#     find_path(CURL_INCLUDE_DIR
#         NAMES curl/curl.h
#         PATHS ${CURL_SEARCH_PATHS}
#         PATH_SUFFIXES include
#         DOC "CURL include directory"
#     )
    
#     find_library(CURL_LIBRARY
#         NAMES curl libcurl
#         PATHS ${CURL_SEARCH_PATHS}
#         PATH_SUFFIXES lib lib64
#         DOC "CURL library"
#     )
    
#     if(CURL_INCLUDE_DIR AND CURL_LIBRARY)
#         set(CURL_FOUND TRUE)
#         set(CURL_INCLUDE_DIRS ${CURL_INCLUDE_DIR})
#         set(CURL_LIBRARIES ${CURL_LIBRARY})
#         message(STATUS "✓ Found libcurl manually")
#         message(STATUS "  Include dir: ${CURL_INCLUDE_DIR}")
#         message(STATUS "  Library: ${CURL_LIBRARY}")
#     else()
#         message(STATUS "✗ Manual search failed")
#     endif()
# endif()

if(NOT CURL_FOUND)
    if(WIN32)
        set(CURL_INCLUDE_DIR "C:/msys64/mingw64/include")
        set(CURL_LIBRARY "C:/msys64/mingw64/lib/libcurl.dll.a")
        if(EXISTS ${CURL_INCLUDE_DIR} AND EXISTS ${CURL_LIBRARY})
            message(STATUS "Using manual CURL paths")
        else()
            message(FATAL_ERROR "CURL not found and manual paths don't exist")
        endif()
    else()
        message(FATAL_ERROR "CURL not found. Please install libcurl development package.")
    endif()
endif()

# ========== Add ImGui ==========
FetchContent_Declare(imgui_source
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.91.0
    GIT_SHALLOW ON
    EXCLUDE_FROM_ALL
    SYSTEM
)

FetchContent_Declare(GLFW
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
    GIT_SHALLOW ON
    EXCLUDE_FROM_ALL
    SYSTEM
)

FetchContent_MakeAvailable(imgui_source GLFW)

# Create ImGui library
add_library(imgui STATIC
    ${imgui_source_SOURCE_DIR}/imgui.cpp
    ${imgui_source_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_source_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_source_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_source_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_source_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_source_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

target_include_directories(imgui PUBLIC
    ${imgui_source_SOURCE_DIR}
    ${imgui_source_SOURCE_DIR}/backends
)

target_link_libraries(imgui PUBLIC
    glfw
    OpenGL::GL
)

# Include directories
if(CURL_FOUND)
    target_include_directories(imgui INTERFACE ${CURL_INCLUDE_DIRS})
else()
    include_directories(${CURL_INCLUDE_DIR})
endif()

# Add your application sources
file(GLOB APP_SOURCES application/*.cpp)

# Client
add_executable(Client Client/client.cpp ${APP_SOURCES})
target_link_libraries(Client PRIVATE
    imgui
    ${OpenCV_LIBS}
)

# Server  
add_executable(Server Server/server.cpp ${APP_SOURCES})
target_link_libraries(Server PRIVATE
    imgui
    ${OpenCV_LIBS}
)

# Add CURL and Windows-specific libraries
if(WIN32)
    target_link_libraries(Client PRIVATE 
        ${CURL_LIBRARY} ws2_32 wldap32)
    target_link_libraries(Server PRIVATE 
        ${CURL_LIBRARY} ws2_32 wldap32)
else()
    target_link_libraries(Client PRIVATE ${CURL_LIBRARIES})
    target_link_libraries(Server PRIVATE ${CURL_LIBRARIES})
endif()