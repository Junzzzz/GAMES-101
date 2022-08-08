cmake_minimum_required(VERSION 3.12)

add_executable(Transformation main.cpp)
target_link_libraries(Transformation PRIVATE Eigen3::Eigen)
