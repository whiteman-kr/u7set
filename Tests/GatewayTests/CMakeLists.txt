cmake_minimum_required(VERSION 3.21)

project (GatewayTests LANGUAGES CXX)

set(GOOGLETEST_DIR $ENV{GOOGLETEST_DIR})

find_package(Threads REQUIRED)
find_package(GTest CONFIG REQUIRED)

set(TARGET GatewayTests)

add_executable(${TARGET} 
	./src/AdsGatewayTests.cpp
	./src/AdsGatewayTestsNoAds.cpp
	./src/GwCrc32Tests.cpp
	./src/HashTests.cpp
	./src/main.cpp
	./src/TestSettings.hpp
	./src/TuningGatewayTests.cpp
	./src/TuningGatewayTestsNoTuningService.cpp
)

target_link_libraries(${TARGET} PRIVATE Threads::Threads GTest::gtest GTest::gmock)
target_link_libraries(${TARGET} PRIVATE GatewayClientLib)

add_code_coverage_options(${TARGET} CODE_COVERAGE)