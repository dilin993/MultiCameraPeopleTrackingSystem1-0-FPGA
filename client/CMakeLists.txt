cmake_minimum_required(VERSION 3.5)
project(client)

set(CMAKE_CXX_STANDARD 11)
find_package( OpenCV REQUIRED )
find_package( Boost REQUIRED COMPONENTS system)
include_directories( ${OpenCV_INCLUDE_DIRS})
include_directories( ${Boost_INCLUDE_DIR})


file(GLOB SOURCE_FILES
        "src/*.h"
        "src/*.cpp"
        "src/*.hpp"
        )


add_executable(client ${SOURCE_FILES})

target_link_libraries( client ${OpenCV_LIBS} ${Boost_LIBRARIES} -lpthread -lboost_serialization)
