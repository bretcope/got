cmake_minimum_required(VERSION 2.4)
project(got)

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/out")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/out")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/out")

if (WIN32)
    add_definitions(-DWIN32)
elseif(UNIX)
    add_definitions(-DUNIX)
endif()

add_executable(got src/main.cpp src/Token.h src/Node.cpp src/Node.h src/GotString.cpp src/GotString.h src/Token.cpp src/Lexer.cpp src/Lexer.h src/FileReader.cpp src/FileReader.h src/Utf8Char.h)