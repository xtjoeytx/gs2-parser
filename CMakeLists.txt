cmake_minimum_required(VERSION 3.10)
project(gs2test VERSION 1.0)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(BISON REQUIRED)
find_package(FLEX REQUIRED)

if(WIN32)
  set(FLEX_FLAGS "--wincompat")
endif(WIN32)

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

		src/ast/ast.h
		src/ast/astvisitor.h
		src/ast/astnodevisitor.h
		src/ast/expressiontypes.h
		src/encoding/buffer.h
		src/encoding/graalencoding.h
		src/exceptions/GS2CompilerError.h
		src/utils/ContextThreadPool.h
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
		${FLEX_GS2Scanner_OUTPUTS})

add_executable(gs2test ${SOURCES_ALL} src/main.cpp)
add_library(gs2compiler ${SOURCES_ALL})
target_link_libraries(gs2test)
target_link_libraries(gs2compiler)

set(GS2COMPILER_INCLUDE_DIRECTORY
		"${PROJECT_SOURCE_DIR}/include"
		"${PROJECT_BINARY_DIR}/include"
		"${PROJECT_SOURCE_DIR}/src"
		PARENT_SCOPE)
