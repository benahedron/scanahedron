cmake_minimum_required(VERSION 3.11)
set( CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/contrib/cmake"  ${CMAKE_MODULE_PATH})

option(BUILD_TESTS "Build test programs" OFF)

##############################
### The project
##############################
project(libscanahedron)

### Dependencies:
# libsane
find_package(Sane REQUIRED)
find_package(PNG REQUIRED)
find_package(PNG++ REQUIRED)

### The sources
file(GLOB_RECURSE SOURCE_FILES "src/*.cpp" "src/*.h")
add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES})

### Node specific stuff
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "" SUFFIX ".node")

### Include (incl. node specific stuff)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_JS_INC} ${SANE_INCLUDE_DIR} ${PNG_INCLUDE_DIRS} ${PNG++_INCLUDE_DIR}  "${CMAKE_SOURCE_DIR}/src")

### Link the project properly.
target_link_libraries(${PROJECT_NAME} ${CMAKE_JS_LIB} ${SANE_LIBRARIES} ${PNG_LIBRARIES})


if(BUILD_TESTS)
    ##############################
    ### E2E tests (Small applications)
    ##############################
    project(test_scanner)

    add_definitions("-DNO_NODE")

    ### The sources
    add_executable(${PROJECT_NAME} ${SOURCE_FILES} "tests/e2e/sanescanner.cpp")

    ### Include (incl. node specific stuff)
    target_include_directories(${PROJECT_NAME} PRIVATE ${SANE_INCLUDE_DIR} ${PNG_INCLUDE_DIRS} ${PNG++_INCLUDE_DIR})

    ### Link the project properly.
    target_link_libraries(${PROJECT_NAME} ${CMAKE_JS_LIB} ${SANE_LIBRARIES} ${PNG_LIBRARIES})


    ##############################
    ### Unit Tests (GTest Based)
    ##############################
    find_package(GTest REQUIRED)
    include(CTest)
    include_directories(${GTEST_INCLUDE_DIRS} ${SANE_INCLUDE_DIR} "${CMAKE_SOURCE_DIR}/src")

    add_definitions("-DNO_NODE")

    ### Test files
    file(GLOB_RECURSE TEST_SOURCE_FILES "tests/unit/*.cpp" "tests/unit/*.h")

    ### Create a gtest runner
    add_executable(tests ${TEST_SOURCE_FILES} ${SOURCE_FILES})
    target_include_directories(tests PRIVATE ${SANE_INCLUDE_DIR} ${PNG_INCLUDE_DIRS} ${PNG++_INCLUDE_DIR})
    target_link_libraries(tests GTest::GTest gmock_main ${SANE_LIBRARIES} ${PNG_LIBRARIES} )

    gtest_discover_tests(tests)
    add_test(NAME monolithic COMMAND tests)

endif(BUILD_TESTS)