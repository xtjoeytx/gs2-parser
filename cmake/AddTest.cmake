function(add_test_og TARGET_NAME TARGET_PATH)
    cmake_minimum_required(VERSION 3.22)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})

    file(GLOB_RECURSE TESTS "${TARGET_PATH}/${TARGET_NAME}/*.cpp")

    message(STATUS "Test: ${TARGET_NAME}")
    add_executable(${TARGET_NAME} ${TESTS})
    target_link_libraries(${TARGET_NAME} PRIVATE ${APP_LIBRARY_NAME} Catch2::Catch2WithMain)
    target_link_options(${TARGET_NAME} PRIVATE -static -fstack-protector)
    target_link_libraries(${TARGET_NAME} PUBLIC -static-libgcc -static-libstdc++)

    target_include_directories(${TARGET_NAME} PUBLIC ${PROJECT_BINARY_DIR}/src)

    target_include_directories(${TARGET_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/src)

    add_dependencies(${TARGET_NAME} gs2compiler)

    list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)

    include(CTest)
    include(Catch)
    catch_discover_tests(${TARGET_NAME})

    message(STATUS "Added test ${TARGET_NAME}")
endfunction()