cmake_minimum_required(VERSION 3.10)
project(gs2test VERSION 1.0)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(BISON REQUIRED)
find_package(FLEX REQUIRED)

if(WIN32)
  set(FLEX_FLAGS "--wincompat")
endif(WIN32)

BISON_TARGET(MyParser generator/calc.y ${CMAKE_CURRENT_BINARY_DIR}/calc.tab.cc)
FLEX_TARGET(MyScanner generator/calc.l ${CMAKE_CURRENT_BINARY_DIR}/lex.yy.cc DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/lex.yy.h COMPILE_FLAGS "${FLEX_FLAGS}")
ADD_FLEX_BISON_DEPENDENCY(MyScanner MyParser)

include_directories(
	${CMAKE_CURRENT_BINARY_DIR}
	src
)

set(SOURCES_ALL
		src/encoding/buffer.cpp
		src/ast.cpp
		src/GS2Bytecode.cpp
		src/GS2CompilerVisitor.cpp
		src/Parser.cpp

		src/encoding/buffer.h
		src/encoding/graalencoding.h
		src/ast.h
		src/astvisitor.h
		src/GS2Bytecode.h
		src/GS2CompilerVisitor.h
		src/GS2SourceVisitor.h
		src/opcodes.h
		src/Parser.h

		${BISON_MyParser_INPUT}
		${FLEX_MyScanner_INPUT}
		${BISON_MyParser_OUTPUTS}
		${FLEX_MyScanner_OUTPUTS})

add_executable(gs2test ${SOURCES_ALL} src/main.cpp)
add_library(gs2compiler ${SOURCES_ALL})
target_link_libraries(gs2test)
target_link_libraries(gs2compiler)

set(GS2COMPILER_INCLUDE_DIRECTORY
		"${PROJECT_SOURCE_DIR}/include"
		"${PROJECT_BINARY_DIR}/include"
		"${PROJECT_SOURCE_DIR}/src"
		PARENT_SCOPE)
