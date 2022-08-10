cmake_minimum_required(VERSION 3.12)
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

set(VCPKG "C:/Users/Public/dev/vcpkg")
set(CMAKE_TOOLCHAIN_FILE "${VCPKG}/scripts/buildsystems/vcpkg.cmake"
        CACHE STRING "Vcpkg toolchain file")

project (GAMES101 CXX)

if(CMAKE_CL_64)
    message(STATUS "Platform: x64")
    set(PLATFORM "x64-windows")
else()
    message(STATUS "Platform: x86")
    set(PLATFORM "x86-windows")
endif()

set(Eigen3_DIR ${VCPKG}/installed/${PLATFORM}/share/eigen3)
find_package(Eigen3 CONFIG REQUIRED)

if((Eigen3_FOUND) AND (DEFINED EIGEN3_INCLUDE_DIR))
    message(STATUS "Found Eigen3: ${EIGEN3_INCLUDE_DIR}")
    include_directories(${EIGEN3_INCLUDE_DIR})
else()
    message(FATAL_ERROR "Eigen3 not found.")
endif()

set(OpenCV_DIR ${VCPKG}/installed/${PLATFORM}/share/opencv4)
find_package(OpenCV REQUIRED)

if(OpenCV_FOUND)
    message(STATUS "Found OpenCV: ${OpenCV_INCLUDE_DIRS}/opencv2")
else()
    message(FATAL_ERROR "OpenCV not found.")
endif()

add_subdirectory(Assignment-0)
add_subdirectory(Assignment-1)
add_subdirectory(Assignment-2)