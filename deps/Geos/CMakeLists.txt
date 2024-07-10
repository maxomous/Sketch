
cmake_minimum_required(VERSION 2.8.12)

####################################################
#               Project Properties
####################################################

project("Geos"
    DESCRIPTION "A wrapper for GEOS functions"
    LANGUAGES CXX
)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")

option(ENABLE_DEBUG "Enable debug mode" OFF)

####################################################
#          Include Source File Directories 
####################################################

set(DIRS "")

####################################################
#           Create List of Source Files
####################################################

## Add Source Files from the main directory
file(GLOB SRC_FILES *.c *.cpp *.h *.hpp)
## Add Source Files from the other directories
foreach(DIR ${DIRS})
    # Find all source files & append to list
    file(GLOB SRC_FILES_IN_FOLDER ${DIR}/*.c ${DIR}/*.cpp ${DIR}/*.h ${DIR}/*.hpp)
    list(APPEND SRC_FILES ${SRC_FILES_IN_FOLDER})
endforeach()

## Show the files being added & put each file on it's own line
if(1)
    message(STATUS "Compling Project '${PROJECT_NAME}'")
    message(STATUS "Source Files:")
    foreach(SRC_FILE ${SRC_FILES}) 
        message(${SRC_FILE}) 
    endforeach()
endif()




####################################################
#                 Build Target
####################################################

## Build target with source files
add_library(${PROJECT_NAME} STATIC ${SRC_FILES})

####################################################
#                 Link Libraries
####################################################

target_link_libraries(${PROJECT_NAME}
    geos_c
    geos
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
