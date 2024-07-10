
cmake_minimum_required(VERSION 2.8.12)

####################################################
#               Project Properties
####################################################

project("ConstraintSolver"
    DESCRIPTION "A geometric constraints solver wrapper for SolveSpaceLib"
    LANGUAGES CXX
)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(ENABLE_DEBUG "Enable debug mode" OFF)

####################################################
#            Include
####################################################

## Include other cmakefile projects
add_subdirectory(
    libslvs
)

####################################################
#                 Build Target
####################################################

## Build target with source files
add_library(${PROJECT_NAME} 
    solver.cpp
) # add_library / add_executable


####################################################
#                Include Librarys
#################################################### 

target_link_libraries(${PROJECT_NAME}
    PUBLIC libslvs
)

####################################################
#             Set Compiler Options
####################################################

set(DEBUG_COMPILE_OPTIONS -Wall -Wextra -Wpedantic -Wno-psabi)
    
if (ENABLE_DEBUG)
    message(STATUS "Debug Mode Enabled for '${PROJECT_NAME}'")
    list(APPEND DEBUG_COMPILE_OPTIONS -g)
endif ()

target_compile_options(${PROJECT_NAME} PRIVATE ${DEBUG_COMPILE_OPTIONS}) 

####################################################
