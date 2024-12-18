cmake_minimum_required(VERSION 3.10)
project(gs2test VERSION 1.0)
set(CMAKE_VERBOSE_MAKEFILE ON)

set(CMAKE_DEBUG_POSTFIX _d)

# set(BIN_DIR "bin" CACHE STRING "Binary output directory")

# specify the C++ standard
set(CMAKE_CXX_STANDARD 23)
# set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# TODO: @xtjoeytx, do we need this? It breaks the rust build. I think we should leave it
# up to the user of the library to specify how the output should be built.
# Set up output ditrectories
# set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
# set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/${BIN_DIR})

# # Second, for multi-config builds (e.g. msvc)
# foreach( OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES} )
# 	string( TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG )
# 	set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${PROJECT_SOURCE_DIR}/lib )
# 	set( CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${PROJECT_SOURCE_DIR}/lib )
# 	set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${PROJECT_SOURCE_DIR}/${BIN_DIR} )
# endforeach( OUTPUTCONFIG CMAKE_CONFIGURATION_TYPES )

link_directories(${PROJECT_SOURCE_DIR}/lib)

if(WIN32 OR APPLE)
	set(CMAKE_SHARED_LIBRARY_PREFIX "")
	set(CMAKE_STATIC_LIBRARY_PREFIX "lib")
endif()

# Attempt to find Bison and Flex first
find_package(BISON 3.4 QUIET)
find_package(FLEX QUIET)

# If still not found, fail
if(NOT BISON_FOUND)
	message(FATAL_ERROR "Bison not found. Please install Bison.")
endif()
if(NOT FLEX_FOUND)
	message(FATAL_ERROR "Flex not found. Please install Flex.")
endif()

if(WIN32 AND NOT MINGW)
	set(FLEX_FLAGS "--wincompat")
endif()

# Generate parser and scanner with Bison / Flex
BISON_TARGET(GS2Parser generator/gs2parser.y ${CMAKE_CURRENT_BINARY_DIR}/gs2parser.tab.cc)
FLEX_TARGET(GS2Scanner generator/gs2scanner.l ${CMAKE_CURRENT_BINARY_DIR}/lex.yy.cc DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/lex.yy.h COMPILE_FLAGS "${FLEX_FLAGS}")
ADD_FLEX_BISON_DEPENDENCY(GS2Scanner GS2Parser)

include_directories(
	${CMAKE_CURRENT_BINARY_DIR}
	src
)

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

add_executable(gs2test ${SOURCES_ALL} src/main.cpp)

# Add library (static or shared)
if(STATIC)
	add_library(gs2compiler STATIC ${SOURCES_ALL})
else()
	add_library(gs2compiler SHARED ${SOURCES_ALL})
endif()

# On Windows + MinGW, ensure -fno-rtti is set
# TODO: Why is this necessary? Is it for performance / compatibility reasons?
if(WIN32 AND MINGW)
	target_compile_options(gs2compiler PRIVATE "-fno-rtti")
endif()

# Link fmtlib
#target_link_libraries(gs2test fmt::fmt)
#target_link_libraries(gs2compiler fmt::fmt)

set(GS2COMPILER_INCLUDE_DIRECTORY
	"${PROJECT_SOURCE_DIR}/include"
	"${PROJECT_BINARY_DIR}/include"
	"${PROJECT_SOURCE_DIR}/src"
	PARENT_SCOPE
)
