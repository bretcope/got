cmake_minimum_required(VERSION 2.6.2)
project(mot)

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -pedantic -Werror=return-type")
elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    # Google Test uses TR1 Tuples, which are deprecated in VC++. We're disabling the warning until Google Test fixes it.
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /D_SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING")
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/out")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/out")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/out")

if (WIN32)
    add_definitions(-DWIN32)
elseif(UNIX)
    add_definitions(-DUNIX)
endif()

if(APPLE)
    add_definitions(-DGTEST_USE_OWN_TR1_TUPLE)
    add_definitions(-D__GLIBCXX__)
endif()

# currently, we're only building for little endian, so I'm just hard-coding this
add_definitions(-DMOT_BIG_ENDIAN=0)

add_subdirectory(src)
add_subdirectory(tests)
