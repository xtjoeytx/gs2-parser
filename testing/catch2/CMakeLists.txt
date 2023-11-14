cmake_minimum_required(VERSION 3.22)
project(gs2-parser-catch2)

if(MAKE_TESTS)
    include(AddTest)
    include(SubdirList)

    # Recursively find all .cpp files in the 'testing' directory.
    file(GLOB_RECURSE TEST_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp)

    # For each test source file
    foreach(TEST_FILE ${TEST_SOURCES})
        # Get the directory of the source file
        get_filename_component(TEST_DIR "${TEST_FILE}" DIRECTORY)

        # Replace directory separator to create a unique test target name
        string(REPLACE "/" "_" TEST_TARGET_NAME "${TEST_DIR}")

        # Append the filename to the target name for uniqueness
        get_filename_component(FILE_NAME "${TEST_FILE}" NAME_WE)
        set(TEST_TARGET_NAME "Catch2_${TEST_TARGET_NAME}_${FILE_NAME}")

        # Create an executable for each test source file
        add_executable(${TEST_TARGET_NAME} ${TEST_FILE})

        # Link the executable with the Catch2 testing library
        target_link_libraries(${TEST_TARGET_NAME} PRIVATE Catch2::Catch2WithMain)

        # Add the test to CTest's list of tests
        add_test(NAME ${TEST_TARGET_NAME} COMMAND ${TEST_TARGET_NAME})
    endforeach()
endif()