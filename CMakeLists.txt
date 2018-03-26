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

add_executable(got
        src/main.cpp
        src/FileContent.cpp
        src/FileContent.h
        src/FileReader.cpp
        src/FileReader.h
        src/FileSpan.cpp
        src/FileSpan.h
        src/GotString.cpp
        src/GotString.h
        src/Lexer.cpp
        src/Lexer.h
        src/Nodes.h
        src/nodes/FileNode.cpp
        src/nodes/Node.cpp
        src/nodes/PropertyListNode.cpp
        src/nodes/PropertyNode.cpp
        src/nodes/PropertyDeclarationNode.cpp
        src/nodes/PropertyBlockNode.cpp
        src/nodes/PropertyValueNode.cpp
        src/Parser.cpp
        src/Parser.h
        src/SyntaxElement.h
        src/Token.cpp
        src/Token.h
        src/Utf8Char.h)