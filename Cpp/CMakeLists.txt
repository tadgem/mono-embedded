cmake_minimum_required(VERSION 3.0.0)
project(mono-embedded VERSION 0.1.0)

# Must have MONO_DIR set as an envionment variable
# TODO: create python setup script

set(MONO_ROOT $ENV{MONO_PATH})
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake-etc)

message("Mono Root : ${MONO_ROOT}")
find_package(mono)

message("Found Mono : ${MONO_FOUND}")
message("MCS Location : ${MCS_EXECUTABLE}")

message("Mono Libraries Location : ${MONO_LIBRARIES}")
message("Mono Library Dirs : ${MONO_LIBRARY_DIRS}")
message("Mono Include Dirs : ${MONO_INCLUDE_DIRS}")
message("Required Flags : ${MONO_CFLAGS}")


add_executable(${PROJECT_NAME} main.cpp MonoUtils.hpp)

target_include_directories(${PROJECT_NAME} PUBLIC ${MONO_INCLUDE_DIRS})
target_link_directories(${PROJECT_NAME} PUBLIC ${MONO_LIBRARY_DIRS})
target_link_libraries(${PROJECT_NAME} ${MONO_LIBRARIES})