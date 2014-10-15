cmake_minimum_required(VERSION 2.8)

project(tgs-index)

### Flags for all
SET(CMAKE_CXX_FLAGS "-Wall")

### CXX FLAGS
include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
if(COMPILER_SUPPORTS_CXX11)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif(COMPILER_SUPPORTS_CXX0X)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
else()
        message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()



### Default parameter for debug
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Og -DDEBUG")
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
   message(STATUS "Makefile for Debug mode")
endif()

### Default parametrs for release
#set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")

### libcds static
include_directories(../libcds/includes)
ADD_LIBRARY(libcds STATIC IMPORTED)
SET_TARGET_PROPERTIES(libcds PROPERTIES
    IMPORTED_LOCATION  ${CMAKE_CURRENT_SOURCE_DIR}/../libcds/lib/libcds.a)

### wtmatrix static
include_directories(../wtmatrix/)
ADD_LIBRARY(wtmatrix STATIC IMPORTED)
SET_TARGET_PROPERTIES(wtmatrix PROPERTIES
    IMPORTED_LOCATION  ${CMAKE_CURRENT_SOURCE_DIR}/../wtmatrix/wavelet_matrix.o)

### Common objects files
add_library(Objs OBJECT tgs.cpp mywtmatrix.cpp arraysort.cpp)

add_executable(create create.cpp $<TARGET_OBJECTS:Objs>)
target_link_libraries(create libcds wtmatrix)
 
add_executable(benchmark benchmark.cpp  $<TARGET_OBJECTS:Objs>)
target_link_libraries(benchmark libcds wtmatrix)

add_executable(benchmark2 benchmark2.cpp  $<TARGET_OBJECTS:Objs>)
target_link_libraries(benchmark2 libcds wtmatrix)

add_executable(exp-query exp-query.cpp  $<TARGET_OBJECTS:Objs>)
target_link_libraries(exp-query libcds wtmatrix)

#install(TARGETS create DESTINATION bin)
