cmake_minimum_required(VERSION 3.10)
project(gs2test VERSION 1.0)

# Enable verbose makefile for debugging purposes
set(CMAKE_VERBOSE_MAKEFILE ON)

# Set debug postfix for binaries
set(CMAKE_DEBUG_POSTFIX _d)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Include necessary directories
include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    src
)

# Adjust library prefixes for specific platforms
if(WIN32 OR APPLE)
    set(CMAKE_SHARED_LIBRARY_PREFIX "")
    set(CMAKE_STATIC_LIBRARY_PREFIX "lib")
endif()

# Handle winflexbison setup on Windows (not MinGW)
if(WIN32 AND NOT MINGW)
	execute_process(COMMAND ${CMAKE_COMMAND} -S${CMAKE_CURRENT_SOURCE_DIR}/dependencies/winflexbison -B${CMAKE_CURRENT_SOURCE_DIR}/dependencies/winflexbison/build-winflex-bison -GNinja -DCMAKE_BUILD_TYPE=Release)
	execute_process(COMMAND ${CMAKE_COMMAND} --build ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/winflexbison/build-winflex-bison --parallel 8)
	LIST(APPEND CMAKE_PROGRAM_PATH ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/winflexbison/bin/Release)
	set(FLEX_FLAGS "--wincompat")
endif(WIN32 AND NOT MINGW)

find_package(BISON 3.4 REQUIRED)
find_package(FLEX REQUIRED)

# Check for Bison and Flex availability
if(NOT BISON_EXECUTABLE AND (WIN32 AND NOT MINGW))
    message(FATAL_ERROR "Bison not found. Please ensure winflexbison is available.")
endif()

if(NOT FLEX_EXECUTABLE AND (WIN32 AND NOT MINGW))
    message(FATAL_ERROR "Flex not found. Please ensure winflexbison is available.")
endif()

# Add Flex compatibility flags for Windows
if(WIN32 AND NOT MINGW)
    set(FLEX_FLAGS "--wincompat")
endif()

# Generate parser and scanner with Bison/Flex
BISON_TARGET(GS2Parser generator/gs2parser.y ${CMAKE_CURRENT_BINARY_DIR}/gs2parser.tab.cc)
FLEX_TARGET(GS2Scanner generator/gs2scanner.l ${CMAKE_CURRENT_BINARY_DIR}/lex.yy.cc
    DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/lex.yy.h
    COMPILE_FLAGS "${FLEX_FLAGS}")
ADD_FLEX_BISON_DEPENDENCY(GS2Scanner GS2Parser)

# List source files
set(SOURCES_ALL
    src/ast/ast.cpp
    src/encoding/buffer.cpp
    src/visitors/GS2CompilerVisitor.cpp
    src/GS2BuiltInFunctions.cpp
    src/GS2Bytecode.cpp
    src/GS2Context.cpp
    src/Parser.cpp
    src/c_interface.cpp

    src/ast/ast.h
    src/ast/astvisitor.h
    src/ast/astnodevisitor.h
    src/ast/expressiontypes.h
    src/encoding/buffer.h
    src/encoding/graalencoding.h
    src/utils/EventHandler.h
    src/exceptions/GS2CompilerError.h
    src/utils/ContextThreadPool.h
    src/utils/format_string.h
    src/visitors/FunctionInspectVisitor.h
    src/visitors/GS2CompilerVisitor.h
    src/visitors/GS2SourceVisitor.h
    src/CompilerThreadJob.h
    src/GS2BuiltInFunctions.h
    src/GS2Bytecode.h
    src/GS2Context.h
    src/opcodes.h
    src/Parser.h

    ${BISON_GS2Parser_INPUT}
    ${FLEX_GS2Scanner_INPUT}
    ${BISON_GS2Parser_OUTPUTS}
    ${FLEX_GS2Scanner_OUTPUTS}
)

# Create executable target
add_executable(gs2test ${SOURCES_ALL} src/main.cpp)

# Add library target
if(STATIC)
    add_library(gs2compiler STATIC ${SOURCES_ALL})
else()
    add_library(gs2compiler SHARED ${SOURCES_ALL})
endif()

# Set special compile options for MinGW
if(WIN32 AND MINGW)
    target_compile_options(gs2compiler PRIVATE "-fno-rtti")
endif()

# Export include directory for use by other projects
set(GS2COMPILER_INCLUDE_DIRECTORY
    "${PROJECT_SOURCE_DIR}/include"
    "${PROJECT_BINARY_DIR}/include"
    "${PROJECT_SOURCE_DIR}/src"
    PARENT_SCOPE
)
