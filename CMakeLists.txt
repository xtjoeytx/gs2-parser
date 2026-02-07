cmake_minimum_required(VERSION 3.14)
project(gs2parser VERSION 1.0)
include(GenerateExportHeader)

set(CMAKE_DEBUG_POSTFIX _d)

set(BIN_DIR "bin" CACHE STRING "Binary output directory")

# specify the C++ standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Set up output directories in the build tree
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${BIN_DIR})

# Second, for multi-config builds (e.g. msvc)
foreach( OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES} )
	string( TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG )
	set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR}/lib )
	set( CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR}/lib )
	set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR}/${BIN_DIR} )
endforeach( OUTPUTCONFIG CMAKE_CONFIGURATION_TYPES )

if(WIN32 OR APPLE)
	set(CMAKE_SHARED_LIBRARY_PREFIX "")
	set(CMAKE_STATIC_LIBRARY_PREFIX "lib")
endif()

if(APPLE)
	set(CMAKE_OSX_DEPLOYMENT_TARGET "13.3")
endif()

if(WIN32 AND NOT MINGW)
	execute_process(COMMAND ${CMAKE_COMMAND} -S${CMAKE_CURRENT_SOURCE_DIR}/dependencies/winflexbison -B${CMAKE_CURRENT_SOURCE_DIR}/dependencies/winflexbison/build-winflex-bison -GNinja -DCMAKE_BUILD_TYPE=Release)
	execute_process(COMMAND ${CMAKE_COMMAND} --build ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/winflexbison/build-winflex-bison --parallel 8)
	LIST(APPEND CMAKE_PROGRAM_PATH ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/winflexbison/bin/Release)
	set(FLEX_FLAGS "--wincompat")
endif(WIN32 AND NOT MINGW)

find_package(BISON 3.4 REQUIRED)
find_package(FLEX REQUIRED)

BISON_TARGET(GS2Parser src/parser/gs2parser.y ${CMAKE_CURRENT_BINARY_DIR}/gs2parser.tab.cc)
FLEX_TARGET(GS2Scanner src/parser/gs2scanner.l ${CMAKE_CURRENT_BINARY_DIR}/lex.yy.cc DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/lex.yy.h COMPILE_FLAGS "${FLEX_FLAGS}")
ADD_FLEX_BISON_DEPENDENCY(GS2Scanner GS2Parser)

set(SOURCES
	# AST
	src/ast/ast.cpp

	# Encoding
	src/encoding/buffer.cpp

	# Codegen
	src/codegen/GS2Bytecode.cpp

	# Compiler
	src/compiler/GS2BuiltInFunctions.cpp
	src/compiler/GS2CompilerVisitor.cpp
	src/compiler/GS2Context.cpp

	# Parser
	src/parser/Parser.cpp
	${BISON_GS2Parser_OUTPUTS}
	${FLEX_GS2Scanner_OUTPUTS}
)

set(HEADERS
	# AST
	src/ast/ast.h
	src/ast/expressiontypes.h
	src/ast/NodeVisitor.h

	# Encoding
	src/encoding/buffer.h
	src/encoding/graalencoding.h

	# Codegen
	src/codegen/GS2Bytecode.h

	# Compiler
	src/compiler/GS2BuiltInFunctions.h
	src/compiler/GS2CompilerVisitor.h
	src/compiler/GS2Context.h

	# Parser
	src/parser/Parser.h
	${BISON_GS2Parser_INPUT}
	${FLEX_GS2Scanner_INPUT}

	# Memory
	src/memory/ArenaAllocator.h

	# Visitors
	src/visitors/ASTNodeVisitor.h
	src/visitors/FunctionInspectVisitor.h
	src/visitors/GS2SourceVisitor.h

	# Utils / Misc
	src/CompilerThreadJob.h
	src/exceptions/GS2CompilerError.h
	src/opcodes.h
	src/utils/ContextThreadPool.h
	src/utils/EventHandler.h
)

set(SOURCES_ALL ${SOURCES} ${HEADERS})

# Use standard BUILD_SHARED_LIBS; Emscripten doesn't support shared libs
if(EMSCRIPTEN)
	set(BUILD_SHARED_LIBS OFF)
endif()
option(BUILD_SHARED_LIBS "Build shared libraries" ON)

add_library(gs2compiler ${SOURCES_ALL} bindings/c/c_interface.cpp)
generate_export_header(gs2compiler
	BASE_NAME GS2COMPILER
	EXPORT_FILE_NAME ${CMAKE_CURRENT_BINARY_DIR}/gs2compiler_export.h
)

target_include_directories(gs2compiler
	PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
		$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}/src/parser
		${CMAKE_CURRENT_SOURCE_DIR}/src/codegen
		${CMAKE_CURRENT_SOURCE_DIR}/src/compiler
		${CMAKE_CURRENT_SOURCE_DIR}/src/encoding
		${CMAKE_CURRENT_SOURCE_DIR}/src/memory
)

if(NOT BUILD_SHARED_LIBS)
	target_compile_definitions(gs2compiler PUBLIC GS2COMPILER_STATIC_DEFINE)
endif()

if(WIN32 AND MINGW)
	target_compile_options(gs2compiler PRIVATE "-fno-rtti")
endif()

if(EMSCRIPTEN)
	add_executable(gs2test bindings/js/js_interface.cpp)
	set_target_properties(gs2test PROPERTIES LINK_FLAGS "--embind-emit-tsd=gs2test.d.ts -s ENVIRONMENT=web -s DYNAMIC_EXECUTION=0 -s SINGLE_FILE=1 -s MODULARIZE -s 'EXPORT_NAME=GS2Compiler' --bind")
else()
	add_executable(gs2test src/main.cpp)
endif()
target_link_libraries(gs2test PRIVATE gs2compiler)

# Test suite integration
# Only configure tests if this is the main project (not a subproject)
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
	# Enable testing
	enable_testing()

	# Find Python 3 for test runner
	find_package(Python3 COMPONENTS Interpreter)

	if(Python3_FOUND)
		# Create output directories in build tree (only needed once)
		file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/tests/outputs)
		file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/tests/reports)

		# Custom target: Run all tests with verbose output
		add_custom_target(run-tests
			COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tests/tools/run_tests.py
				--project-root ${CMAKE_CURRENT_SOURCE_DIR}
				--scripts-dir ${CMAKE_CURRENT_SOURCE_DIR}/tests/scripts
				--baselines-dir ${CMAKE_CURRENT_SOURCE_DIR}/tests/baselines
				--output-dir ${CMAKE_BINARY_DIR}/tests/outputs
				--reports-dir ${CMAKE_BINARY_DIR}/tests/reports
			WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
			COMMENT "Running test suite"
			DEPENDS gs2test
		)

		# Custom target: Generate/update baselines
		add_custom_target(test-baselines
			COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tests/tools/run_tests.py
				--project-root ${CMAKE_CURRENT_SOURCE_DIR}
				--scripts-dir ${CMAKE_CURRENT_SOURCE_DIR}/tests/scripts
				--baselines-dir ${CMAKE_CURRENT_SOURCE_DIR}/tests/baselines
				--output-dir ${CMAKE_BINARY_DIR}/tests/outputs
				--reports-dir ${CMAKE_BINARY_DIR}/tests/reports
				--update-baselines
			WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
			COMMENT "Generating test baselines"
			DEPENDS gs2test
		)

		# Custom target: Clean test artifacts
		add_custom_target(test-clean
			COMMAND ${CMAKE_COMMAND} -E rm -rf ${CMAKE_BINARY_DIR}/tests/outputs
			COMMAND ${CMAKE_COMMAND} -E rm -rf ${CMAKE_BINARY_DIR}/tests/reports
			COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/tests/outputs
			COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/tests/reports
			WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
			COMMENT "Cleaning test artifacts"
		)

		# CTest integration for CI/CD
		add_test(
			NAME regression_tests
			COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tests/tools/run_tests.py
				--project-root ${CMAKE_CURRENT_SOURCE_DIR}
				--scripts-dir ${CMAKE_CURRENT_SOURCE_DIR}/tests/scripts
				--baselines-dir ${CMAKE_CURRENT_SOURCE_DIR}/tests/baselines
				--output-dir ${CMAKE_BINARY_DIR}/tests/outputs
				--reports-dir ${CMAKE_BINARY_DIR}/tests/reports
				--quiet
			WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
		)

		# Set test properties
		set_tests_properties(regression_tests PROPERTIES
			TIMEOUT 60
			FAIL_REGULAR_EXPRESSION "Regressions detected"
		)

		message(STATUS "Test suite configured")
		message(STATUS "  Scripts in: ${CMAKE_CURRENT_SOURCE_DIR}/tests/scripts")
		message(STATUS "  Baselines in: ${CMAKE_CURRENT_SOURCE_DIR}/tests/baselines")
		message(STATUS "  Outputs in: ${CMAKE_BINARY_DIR}/tests/outputs")
		message(STATUS "  Reports in: ${CMAKE_BINARY_DIR}/tests/reports")
		message(STATUS "  Available targets:")
		message(STATUS "    run-tests      - Run test suite with verbose output")
		message(STATUS "    test-baselines - Generate/update baseline bytecode")
		message(STATUS "    test-clean     - Clean test artifacts")
		message(STATUS "    test or ctest  - Run tests (quiet, for CI/CD)")

	else()
		message(WARNING "Python 3 not found. Test suite will not be available.")
	endif()
endif()
