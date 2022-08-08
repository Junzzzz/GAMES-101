cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)

add_executable(Rasterizer main.cpp rasterizer.hpp rasterizer.cpp Triangle.hpp Triangle.cpp)
target_link_libraries(Rasterizer ${OpenCV_LIBRARIES})
