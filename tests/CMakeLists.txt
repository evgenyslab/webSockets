cmake_minimum_required(VERSION 3.0.0)

# tests using Google Test.
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

include_directories(
        include
        usr/include
        /usr/local/include
        ../external/uWebSockets/src
)

# get all tests
file(GLOB TESTS *.cpp)

file(GLOB UWS_SOURCES
        "../external/uWebSockets/src/*.c*"
        )


file(GLOB UWS_HEADERS
        "../include/*.h"
        )

# loop over each test
foreach(test_source ${TESTS})
    # strip .cpp extension:
    get_filename_component(test_name ${test_source} NAME)
    string( REPLACE ".cpp" "" test_file_name ${test_name})


    add_executable(${test_file_name} ${test_source} ${UWS_SOURCES} ${UWS_HEADERS})
    target_link_libraries(${test_file_name}  crypto z uv ssl pthread ${GTEST_LIBRARIES})

endforeach(test_source ${TESTS})