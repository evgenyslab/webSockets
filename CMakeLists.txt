cmake_minimum_required(VERSION 3.0.0)
project(websockets_testing)
add_compile_options(-std=c++11)



include_directories(
        include
        usr/include
        /usr/local/include
        external/msgpack/include
        external/uWebSockets/src
#        /usr/local/Cellar/opencv/4.2.0_1/include/opencv4
)

link_directories(/usr/local/lib)
#link_directories(/usr/local/Cellar/opencv/4.2.0_1/lib)

#FIND_PACKAGE(opencv)
#
#MESSAGE(${OPENCV_ROOT})

# macOS only:
if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

    include_directories(/usr/local/opt/openssl/include)
    link_directories(/usr/local/opt/openssl/lib)

endif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")


file(GLOB UWS_SOURCES
        "external/uWebSockets/src/*.c*"
        )

file(GLOB cppUWS_SOURCES
        "src/*.c"
        )

file(GLOB UWS_COMBINED
        "external/uWebSockets/src/*.c*"
        "src/*.c"
        )

file(GLOB UWS_HEADERS
        "include/*.h"
        )

MESSAGE(${UWS_HEADERS})

#file(GLOB OPENCV_LIBS
#        "/usr/local/Cellar/opencv/4.2.0_1/lib/*.dylib"
#        )

# this is only uWebSocket library
add_library(uwserver STATIC ${UWS_COMBINED} include/uWServer.h)

add_executable(message_server examples/message_server.cpp ${UWS_SOURCES} ${UWS_HEADERS})
# Make sure YourLib is linked to each app
target_link_libraries(message_server crypto z uv ssl pthread)



IF(WITH_EXAMPLES)
    MESSAGE("Compiling with Examples")
    file(GLOB examples
            "examples/*.c*"
            )
    foreach( prog ${examples} )
        # I used a simple string replace, to cut off .cpp.
        get_filename_component(progBase ${prog} NAME)
        string( REPLACE ".cpp" "" file_name ${progBase} )
        add_executable( ${file_name} ${prog} ${UWS_SOURCES} ${UWS_HEADERS})
        # Make sure YourLib is linked to each app
        target_link_libraries(${file_name} crypto z uv ssl pthread)

    endforeach(  prog ${programs}  )

ENDIF()
