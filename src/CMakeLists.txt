project(cmot)

add_library(cmotlib
            io/FileContent.cpp
            io/FileContent.h
            io/FileReader.cpp
            io/FileReader.h
            io/FileSpan.cpp
            io/FileSpan.h
            io/MotString.cpp
            io/MotString.h
            io/Utf8.cpp
            io/Utf8.Gen.cpp
            io/Utf8.h
            parsing/Lexer.cpp
            parsing/Lexer.h
            parsing/Nodes.h
            parsing/nodes/FileNode.cpp
            parsing/nodes/Node.cpp
            parsing/nodes/PropertyListNode.cpp
            parsing/nodes/PropertyNode.cpp
            parsing/nodes/PropertyDeclarationNode.cpp
            parsing/nodes/PropertyBlockNode.cpp
            parsing/nodes/PropertyValueNode.cpp
            parsing/Parser.cpp
            parsing/Parser.h
            parsing/SyntaxElement.h
            parsing/Token.cpp
            parsing/Token.h)

add_executable(cmot cmot.cpp)

target_link_libraries(cmot cmotlib)