cmake_minimum_required(VERSION 3.9.3)

if (NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/cmake/toolchain.cmake)
    include(${CMAKE_SOURCE_DIR}/cmake/toolchain.cmake)
endif ()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")
if (NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
    message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
    file(DOWNLOAD
        "https://raw.githubusercontent.com/conan-io/cmake-conan/v0.8/conan.cmake"
        "${CMAKE_BINARY_DIR}/conan.cmake"
        EXPECTED_MD5 58bc519cadc890b5c33235defadc1176)
endif ()
include(${CMAKE_BINARY_DIR}/conan.cmake)

project(aide VERSION 0.0.1)
conan_cmake_run(
    CONANFILE conanfile.txt
    BASIC_SETUP CMAKE_TARGETS
    BUILD missing
)

set(CMAKE_TOOLCHAIN_FILE ${CONAN_USER_EMSDK_emscripten_root}/cmake/Modules/Platform/Emscripten.cmake)
set(CMAKE_C_COMPILER "")
set(CMAKE_CXX_COMPILER "")
set(CMAKE_AR "")
set(CMAKE_RANLIB "")

include(${CONAN_USER_EMSDK_emscripten_root}/cmake/Modules/Platform/Emscripten.cmake)

set(CMAKE_CXX_FLAGS "-s WASM=1 --bind")
SET(CMAKE_CXX_FLAGS_RELEASE "-O2")

find_package(Protobuf REQUIRED)

set(CMAKE_EXECUTABLE_SUFFIX ".html")
file(GLOB_RECURSE SOURCE_FILES src/*.cpp)

set(PROTO_DEST ${CMAKE_CURRENT_BINARY_DIR}/protos)
file(MAKE_DIRECTORY ${PROTO_DEST})

file(GLOB PROTO_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/protos/*.proto)
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_DEST} ${PROTO_FILES})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CONAN_INCLUDE_DIRS})
link_directories(${CONAN_LIB_DIRS})

add_executable(aide
    ${SOURCE_FILES}
    ${PROTO_SRCS})

set(IMPORT_LIBS
    protobuf
    nghttp2)

target_link_libraries(aide PUBLIC ${IMPORT_LIBS})