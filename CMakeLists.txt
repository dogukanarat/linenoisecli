cmake_minimum_required(VERSION 3.9)
cmake_policy(SET CMP0077 NEW)
cmake_policy(SET CMP0079 NEW)


# Set version
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    execute_process(COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/version.sh VERSION ${INCREMENT_VERSION}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        OUTPUT_VARIABLE CUSTOM_VERSION)
else()
    set(CMAKE_BUILD_TYPE Debug)
    set(CUSTOM_VERSION 0.0.0)
endif()

# Set the project name
project(linenoisecli VERSION ${CUSTOM_VERSION} DESCRIPTION "Simple command line interface using linenoise")

message(STATUS "PROJECT_NAME = ${PROJECT_NAME}")
message(STATUS "CMAKE_BUILD_TYPE = ${CMAKE_BUILD_TYPE}")
message(STATUS "CUSTOM_VERSION = ${CUSTOM_VERSION}")

# Set the C++ standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_VERBOSE_MAKEFILE ON)

# Define compiler flags
add_compile_options(-pedantic -Wall -Wextra)

# GNU standard installation directories
include(GNUInstallDirs)

message("Compiler ID: ${CMAKE_CXX_COMPILER_ID}")
message("Compiler: ${CMAKE_CXX_COMPILER}")
message("Compiler Version: ${CMAKE_CXX_COMPILER_VERSION}")
message("System: ${CMAKE_SYSTEM_NAME}")
message("System Version: ${CMAKE_SYSTEM_VERSION}")
message("System Processor: ${CMAKE_SYSTEM_PROCESSOR}")
message("Build Type: ${CMAKE_BUILD_TYPE}")
message("C Version: ${CMAKE_C_STANDARD}")
message("C++ Version: ${CMAKE_CXX_STANDARD}")

configure_file(${PROJECT_SOURCE_DIR}/include/linenoisecli/linenoisecli_autoconfig.hpp.in
               ${PROJECT_BINARY_DIR}/linenoisecli/linenoisecli_autoconfig.hpp)

set(DEP_INCLUDE_DIRS)
set(DEP_LIBRARIES)

set(SOURCES
    src/linenoisecli.cpp
    src/linenoise.c
    )

set(PUB_HEADERS
    include/linenoisecli/linenoisecli.hpp
    )

add_library(${PROJECT_NAME} SHARED ${SOURCES} ${PUB_HEADERS})

target_include_directories(${PROJECT_NAME} PUBLIC include)
target_include_directories(${PROJECT_NAME} PRIVATE src)
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_BINARY_DIR})
target_include_directories(${PROJECT_NAME} PUBLIC ${DEP_INCLUDE_DIRS})

target_link_libraries(${PROJECT_NAME} PRIVATE ${DEP_LIBRARIES})

set_target_properties(${PROJECT_NAME} PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(${PROJECT_NAME} PROPERTIES SOVERSION ${PROJECT_VERSION_MAJOR})

# Make position independent code for general purpose shared libraries
set_target_properties(${PROJECT_NAME} PROPERTIES POSITION_INDEPENDENT_CODE ON)

# Create pkg-config file
configure_file(${PROJECT_NAME}.pc.in ${PROJECT_NAME}.pc @ONLY)

# Install

install(TARGETS ${PROJECT_NAME}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

install(FILES ${PUB_HEADERS}
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/linenoisecli)

install(FILES ${CMAKE_BINARY_DIR}/linenoisecli/linenoisecli_autoconfig.hpp
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/linenoisecli)

install(FILES ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.pc
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)

# Add example
add_executable(linenoisecli_example ${CMAKE_CURRENT_SOURCE_DIR}/example/main.cpp)
target_include_directories(linenoisecli_example PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/example )
target_include_directories(linenoisecli_example PUBLIC ${INCLUDE})
target_link_libraries(linenoisecli_example PRIVATE linenoisecli)
