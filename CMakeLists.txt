cmake_minimum_required(VERSION 3.28)
project(gs2parser VERSION 1.0)
include(GenerateExportHeader)

if(PROJECT_IS_TOP_LEVEL)
	set(CMAKE_DEBUG_POSTFIX _d)
	set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

	set(BIN_DIR "bin" CACHE STRING "Binary output directory")

	# Set up output directories in the build tree
	set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
	set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${BIN_DIR})

	# Second, for multi-config builds (e.g. msvc)
	foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
		string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG)
		set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR}/lib)
		set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR}/lib)
		set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR}/${BIN_DIR})
	endforeach()

	if(APPLE)
		set(CMAKE_OSX_DEPLOYMENT_TARGET "13.3")
	endif()
endif()

if(WIN32 AND NOT MINGW)
	include(FetchContent)
	FetchContent_Declare(winflexbison
			GIT_REPOSITORY https://github.com/lexxmark/winflexbison.git
			GIT_TAG 2040672b690523e6a5b2e51480d90132a39d4c76
	)
	FetchContent_Populate(winflexbison)
	execute_process(
			COMMAND ${CMAKE_COMMAND}
			-S ${winflexbison_SOURCE_DIR}
			-B ${winflexbison_BINARY_DIR}
			-G Ninja
			-DCMAKE_BUILD_TYPE=Release
	)
	execute_process(
			COMMAND ${CMAKE_COMMAND}
			--build ${winflexbison_BINARY_DIR}
			--parallel
	)
	list(APPEND CMAKE_PROGRAM_PATH ${winflexbison_BINARY_DIR}/bin/Release)
	set(FLEX_FLAGS "--wincompat")
endif()

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

option(BUILD_SHARED_LIBS "Build shared libraries" ON)
if(EMSCRIPTEN)
	set(BUILD_SHARED_LIBS OFF CACHE BOOL "Emscripten doesn't support shared libs" FORCE)
endif()

add_library(gs2compiler ${SOURCES_ALL} bindings/c/c_interface.cpp)
add_library(gs2parser::gs2compiler ALIAS gs2compiler)
generate_export_header(gs2compiler
		BASE_NAME GS2COMPILER
		EXPORT_FILE_NAME ${CMAKE_CURRENT_BINARY_DIR}/gs2compiler_export.h
)

target_compile_features(gs2compiler PUBLIC cxx_std_23)
set_target_properties(gs2compiler PROPERTIES
		POSITION_INDEPENDENT_CODE ON
		CXX_VISIBILITY_PRESET hidden
		VISIBILITY_INLINES_HIDDEN ON
)
if(BUILD_SHARED_LIBS)
	set_target_properties(gs2compiler PROPERTIES PREFIX "")
endif()

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

option(GS2PARSER_BUILD_GS2TEST "Build gs2test executable" ${PROJECT_IS_TOP_LEVEL})

if(GS2PARSER_BUILD_GS2TEST)
	if(EMSCRIPTEN)
		add_executable(gs2test bindings/js/js_interface.cpp)
		set_target_properties(gs2test PROPERTIES LINK_FLAGS "--embind-emit-tsd=gs2test.d.ts -s ENVIRONMENT=web -s DYNAMIC_EXECUTION=0 -s SINGLE_FILE=1 -s MODULARIZE -s 'EXPORT_NAME=GS2Compiler' --bind")
	else()
		add_executable(gs2test src/main.cpp)
	endif()
	target_link_libraries(gs2test PRIVATE gs2compiler)
endif()

# Test suite integration
# Only configure tests if this is the main project (not a subproject)
if(PROJECT_IS_TOP_LEVEL)
	# Enable testing
	enable_testing()

	# Find Python 3 for test runner
	find_package(Python3 COMPONENTS Interpreter)

	if(Python3_FOUND)
		# Create output directories in build tree (only needed once)
		file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/tests/outputs)
		file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/tests/reports)

		set(TEST_RUNNER_CMD
				${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tests/tools/run_tests.py
				--project-root ${CMAKE_CURRENT_SOURCE_DIR}
				--scripts-dir ${CMAKE_CURRENT_SOURCE_DIR}/tests/scripts
				--baselines-dir ${CMAKE_CURRENT_SOURCE_DIR}/tests/baselines
				--output-dir ${CMAKE_BINARY_DIR}/tests/outputs
				--reports-dir ${CMAKE_BINARY_DIR}/tests/reports
		)

		add_custom_target(run-tests
				COMMAND ${TEST_RUNNER_CMD}
				WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
				COMMENT "Running test suite"
				DEPENDS gs2test
		)

		# Custom target: Generate/update baselines
		add_custom_target(test-baselines
				COMMAND ${TEST_RUNNER_CMD} --update-baselines
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
				COMMAND ${TEST_RUNNER_CMD} --quiet
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
