cmake_minimum_required(VERSION 2.8.12)

# Build a static library
add_library(libslvs STATIC 
    constrainteq.cpp
	dsc.h
    entity.cpp
	expr.h
    expr.cpp
	polygon.h
	resource.h
	sketch.h
    solvespace.h
    system.cpp
	util.cpp
    platform/platform.h
    platform/unixutil.cpp
	include/slvs.h
	srf/surface.h
    lib.cpp
	include/SolveSpaceSystem.h
	SolveSpaceSystem.cpp)

target_compile_options(libslvs
    PRIVATE -Wno-missing-field-initializers -Wno-psabi
    PUBLIC -std=c++11)

target_compile_definitions(libslvs
	PRIVATE -DLIBRARY)

target_include_directories(libslvs
    PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include 
    PRIVATE ${CMAKE_CURRENT_LIST_DIR} )

set_target_properties(libslvs PROPERTIES
    PUBLIC_HEADER ${CMAKE_SOURCE_DIR}/include/slvs.h
    VERSION ${solvespace_VERSION_MAJOR}.${solvespace_VERSION_MINOR}
    SOVERSION 1)

