cmake_minimum_required(VERSION 3.21)

project (AdsGatewayTests LANGUAGES CXX)

set(GOOGLETEST_DIR $ENV{GOOGLETEST_DIR})

find_package(Threads REQUIRED)
find_package(GTest CONFIG REQUIRED)

set(TARGET AdsGatewayTests)

add_executable(${TARGET} 
	./src/AdsGatewayTests.cpp
	./src/GwCrc32Tests.cpp
	./src/HashTests.cpp
	./src/main.cpp
	./src/TestSettings.hpp
)

target_link_libraries(${TARGET} PRIVATE Threads::Threads GTest::gtest GTest::gmock)
target_link_libraries(${TARGET} PRIVATE AdsGatewayLib)

add_code_coverage_options(${TARGET} CODE_COVERAGE)