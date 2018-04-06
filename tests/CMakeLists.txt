project(mot_tests)

# Make sure Google Test uses the same C Runtime as the rest of our projects
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Set up Google Test
add_subdirectory("${CMAKE_SOURCE_DIR}/lib/googletest"
                 "${CMAKE_BINARY_DIR}/lib/googletest"
                 EXCLUDE_FROM_ALL
)

include_directories("${gtest_SOURCE_DIR}/include"
                    "${gtest_SOURCE_DIR}"
)

# Include cmot files
include_directories("${CMAKE_SOURCE_DIR}/src")

add_executable(testmot
               testmot.cpp
               StringTests.cpp)

target_link_libraries(testmot gtest gtest_main)
target_link_libraries(testmot cmotlib)
