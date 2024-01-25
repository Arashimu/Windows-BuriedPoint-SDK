cmake_minimum_required(VERSION 3.20)

project(BuriedExamples)

add_definitions(-D_WIN32_WINNT=0x0601)

include_directories(. .. ../src ../src/third_party ../src/third_party/spdlog/include ../src/third_party/boost)

add_executable(buried_example buried_example.cpp)
target_link_libraries(buried_example Buried_static)

add_executable(buried_example_shared buried_example.cpp)
target_link_libraries(buried_example_shared Buried_shared)