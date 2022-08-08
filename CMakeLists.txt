cmake_minimum_required(VERSION 3.12)
project (GAMES101 CXX)

find_package(Eigen3 REQUIRED)

if((Eigen3_FOUND) AND (DEFINED EIGEN3_INCLUDE_DIR))
    message(STATUS "Found Eigen3: " ${EIGEN3_INCLUDE_DIR})
    include_directories(${EIGEN3_INCLUDE_DIR})
else()
    message(FATAL_ERROR "Eigen3 not found.")
endif()

add_subdirectory(Assignment-0)
