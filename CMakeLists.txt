cmake_minimum_required(VERSION 2.4)
project(mot)

#set(CMAKE_CXX_STANDARD 17)

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -pedantic -Werror=return-type")
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/out")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/out")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/out")

if (WIN32)
    add_definitions(-DWIN32)
elseif(UNIX)
    add_definitions(-DUNIX)
endif()

add_executable(cmot
               src/io/FileContent.cpp
               src/io/FileContent.h
               src/io/FileReader.cpp
               src/io/FileReader.h
               src/io/FileSpan.cpp
               src/io/FileSpan.h
               src/io/MotString.cpp
               src/io/MotString.h
               src/io/Utf8.cpp
               src/io/Utf8.Gen.cpp
               src/io/Utf8.h
               src/parsing/Lexer.cpp
               src/parsing/Lexer.h
               src/parsing/Nodes.h
               src/parsing/nodes/FileNode.cpp
               src/parsing/nodes/Node.cpp
               src/parsing/nodes/PropertyListNode.cpp
               src/parsing/nodes/PropertyNode.cpp
               src/parsing/nodes/PropertyDeclarationNode.cpp
               src/parsing/nodes/PropertyBlockNode.cpp
               src/parsing/nodes/PropertyValueNode.cpp
               src/parsing/Parser.cpp
               src/parsing/Parser.h
               src/parsing/SyntaxElement.h
               src/parsing/Token.cpp
               src/parsing/Token.h
               src/main.cpp)