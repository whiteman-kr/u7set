cmake_minimum_required(VERSION 3.16)

project(LicenseLibTests LANGUAGES CXX)

set(GOOGLETEST_DIR $ENV{GOOGLETEST_DIR})

find_package(Threads REQUIRED)

set(TARGET LicenseLibTests)

qt_add_executable(${TARGET} 
	src/main.cpp
	src/RpctLicenseTests.cpp

	src/License.qrc
)

target_precompile_headers(${TARGET} PRIVATE
	<gtest/gtest.h>
	<gmock/gmock.h>
	<gmock/gmock-matchers.h>
)

target_link_libraries(${TARGET} PRIVATE Threads::Threads GTest::gtest_main GTest::gmock)
target_link_libraries(${TARGET} PRIVATE LicenseLib)

add_test(NAME ${TARGET} COMMAND ${TARGET})