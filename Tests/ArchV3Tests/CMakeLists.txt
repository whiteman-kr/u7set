cmake_minimum_required(VERSION 3.21)

project (ArchV3Tests LANGUAGES CXX)

set(GOOGLETEST_DIR $ENV{GOOGLETEST_DIR})

find_package(Threads REQUIRED)
find_package(GTest CONFIG REQUIRED)

set(TARGET ArchV3Tests)

add_executable(${TARGET} 
	./src/main.cpp
)

target_link_libraries(${TARGET} PRIVATE Threads::Threads GTest::gtest GTest::gmock)
target_link_libraries(${TARGET} PRIVATE ArchV3Lib)

add_code_coverage_options(${TARGET} CODE_COVERAGE)