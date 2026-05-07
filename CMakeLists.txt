# minimum required version of CMake
cmake_minimum_required(VERSION 3.10)
# set the project name
project(BitLink)
# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
# add the executable
include_directories(include)
# recursively find all .cpp files in the src directory and add them to the executable
file(GLOB_RECURSE SOURCES "src/*.cpp")
# make the executable
add_executable(BitLink ${SOURCES})
# link windows HTTP library
target_link_libraries(BitLink winhttp)