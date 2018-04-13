project(cmot)

add_library(cmotlib
            io/Console.cpp
            io/Console.h
            io/ConsoleType.h
            io/FileContent.cpp
            io/FileContent.h
            io/FileReader.cpp
            io/FileReader.h
            io/FileSpan.cpp
            io/FileSpan.h
            io/printing/FmtPosition.cpp
            io/printing/FmtPosition.h
            Macros.h
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
            parsing/Token.h
            text/MotString.cpp
            text/MotString.h
            text/Utf8.cpp
            text/Utf8.Gen.cpp
            text/Utf8.Iterator.cpp
            text/Utf8.h)

add_executable(cmot cmot.cpp)

target_link_libraries(cmot cmotlib)