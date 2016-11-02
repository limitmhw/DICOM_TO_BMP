#version
cmake_minimum_required (VERSION 2.8)

#project information

project (DICOM_TO_BMP)

include_directories(./include)

add_subdirectory(./src)

add_executable(run main.cpp)

target_link_libraries(run srclib)
