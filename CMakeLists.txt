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

add_executable(got main.cpp Token.h Node.cpp Node.h GotString.cpp GotString.h Token.cpp Lexer.cpp Lexer.h FileReader.cpp FileReader.h Utf8Char.h)