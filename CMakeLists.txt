cmake_minimum_required(VERSION 3.10)
project(gs2test VERSION 1.0)

set(CMAKE_DEBUG_POSTFIX _d)

set(BIN_DIR "bin" CACHE STRING "Binary output directory")

# specify the C++ standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Set up output directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/${BIN_DIR})

# Second, for multi-config builds (e.g. msvc)
foreach( OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES} )
	string( TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG )
	set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${PROJECT_SOURCE_DIR}/lib )
	set( CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${PROJECT_SOURCE_DIR}/lib )
	set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${PROJECT_SOURCE_DIR}/${BIN_DIR} )
endforeach( OUTPUTCONFIG CMAKE_CONFIGURATION_TYPES )

link_directories(${PROJECT_SOURCE_DIR}/lib)

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

include_directories(
	${CMAKE_CURRENT_BINARY_DIR}
	src
	src/parser
	src/codegen
	src/compiler
	src/encoding
	src/memory
)

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

	# Bindings
	bindings/c/c_interface.cpp
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

if (DEFINED EMSCRIPTEN)
	add_executable(gs2test ${SOURCES_ALL} bindings/js/js_interface.cpp)
	set_target_properties(gs2test PROPERTIES LINK_FLAGS "--embind-emit-tsd=gs2test.d.ts -s ENVIRONMENT=web -s DYNAMIC_EXECUTION=0 -s SINGLE_FILE=1 -s MODULARIZE -s 'EXPORT_NAME=GS2Compiler' --bind")
else()
	add_executable(gs2test ${SOURCES_ALL} src/main.cpp)
endif()

if (STATIC)
	add_library(gs2compiler STATIC ${SOURCES_ALL})
else()
	add_library(gs2compiler SHARED ${SOURCES_ALL})
endif()

if(WIN32 AND MINGW)
	target_compile_options(gs2compiler PRIVATE "-fno-rtti")
endif()

set_property(TARGET gs2test PROPERTY CXX_STANDARD 23)
set_property(TARGET gs2compiler PROPERTY CXX_STANDARD 23)

if(NOT CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
	set(GS2COMPILER_INCLUDE_DIRECTORY
		"${PROJECT_SOURCE_DIR}/include"
		"${PROJECT_BINARY_DIR}/include"
		"${PROJECT_SOURCE_DIR}/src"
		PARENT_SCOPE)
endif()

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
