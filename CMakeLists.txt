cmake_minimum_required(VERSION 3.6)

project(linenoisecli VERSION 0.1 LANGUAGES C CXX)

# set the C++ standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

include(GNUInstallDirs)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR})

message("Compiler ID: ${CMAKE_CXX_COMPILER_ID}")
message("Compiler: ${CMAKE_CXX_COMPILER}")
message("Compiler Version: ${CMAKE_CXX_COMPILER_VERSION}")
message("System: ${CMAKE_SYSTEM_NAME}")
message("System Version: ${CMAKE_SYSTEM_VERSION}")
message("System Processor: ${CMAKE_SYSTEM_PROCESSOR}")
message("Build Type: ${CMAKE_BUILD_TYPE}")
message("C Version: ${CMAKE_C_STANDARD}")
message("C++ Version: ${CMAKE_CXX_STANDARD}")

set(SRC
    src/linenoisecli.cpp
    src/linenoise.c
    )

set(INC
    include/linenoisecli/linenoisecli.hpp
    )


add_library(linenoisecli STATIC ${SRC})
target_include_directories(linenoisecli PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_include_directories(linenoisecli PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(linenoisecli PRIVATE ${LIBS})

add_executable(linenoisecli_example ${CMAKE_CURRENT_SOURCE_DIR}/example/main.cpp)
target_include_directories(linenoisecli_example PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/example )
target_include_directories(linenoisecli_example PUBLIC ${INC})
target_link_libraries(linenoisecli_example PRIVATE linenoisecli)
