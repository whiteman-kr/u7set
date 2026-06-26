cmake_minimum_required(VERSION 3.21)

project (ArchV3Tests LANGUAGES CXX)

set(GOOGLETEST_DIR $ENV{GOOGLETEST_DIR})

find_package(Threads REQUIRED)
find_package(GTest CONFIG REQUIRED)

set(TARGET ArchV3Tests)

add_executable(${TARGET} 
	Common.cpp
	Common.h
	ArchV3TestsMain.cpp
	DbTests.cpp)

 target_compile_definitions(${TARGET} PRIVATE
    _WIN32_WINNT=0x0A00
    WIN32_LEAN_AND_MEAN
    NOMINMAX
)

target_link_libraries(${TARGET} PRIVATE Threads::Threads GTest::gtest GTest::gmock)
target_link_libraries(${TARGET} PRIVATE ArchV3Lib OnlineLib)

add_code_coverage_options(${TARGET} CODE_COVERAGE)