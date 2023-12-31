cmake_minimum_required(VERSION 3.0)
project(TUv)

enable_language(CXX)
set(CMAKE_CXX_STANDARD		17)
set(CMAKE_CXX_STANDARD_REQUIRED	ON)
set(CMAKE_CXX_EXTENSIONS	OFF)
set(CMAKE_CXX_FLAGS_RELEASE	"-Wall -O3")
set(CMAKE_CXX_FLAGS_DEBUG	"-g")
set(CMAKE_BUILD_TYPE		Release)
add_definitions(-DNDEBUG)

set(CMAKE_MODULE_PATH
    ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake_modules")
include(FindX11_Xaw3d)

if(APPLE)
  set(MACOSX_RPATH TRUE)
endif()

find_package(X11 REQUIRED)
find_package(X11_Xaw3d REQUIRED)
find_package(Boost REQUIRED)
include_directories(
    ${PROJECT_SOURCE_DIR}
    ${CMAKE_INSTALL_PREFIX}/include
    ${X11_INCLUDE_DIRS}
    ${Xaw3d_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIRS})

link_directories(${CMAKE_INSTALL_PREFIX}/lib)

file(GLOB sources *.cc *.c)
add_library(${PROJECT_NAME} SHARED ${sources})
target_link_libraries(
    ${PROJECT_NAME}
    TUTools
    ${X11_Xaw3d_LIB}
    ${X11_Xt_LIB}
    ${X11_Xmu_LIB}
    ${X11_LIBRARIES})

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION lib)

add_subdirectory(TU)
add_subdirectory(cmake_modules)
