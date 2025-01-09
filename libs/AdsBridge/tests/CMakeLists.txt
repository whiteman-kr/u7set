cmake_minimum_required(VERSION 3.16)

project(ClientTests LANGUAGES CXX)

set(GOOGLETEST_DIR $ENV{GOOGLETEST_DIR})

find_package(Threads REQUIRED)

set(TARGET AdsBridgeTests)

add_executable(${TARGET}
	src/AdsBridgeTests.cpp
	src/main.cpp
	src/TestSettings.h
)

target_precompile_headers(${TARGET} PRIVATE
	<gtest/gtest.h>
	<gmock/gmock.h>
	<gmock/gmock-matchers.h>
)

# Linux: Sets the runtime search path to $ORIGIN in the installed executable.
#
if (UNIX)
	set_target_properties(${TARGET} PROPERTIES 
		BUILD_RPATH "\$ORIGIN"
		INSTALL_RPATH "\$ORIGIN"
		CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
	set(CMAKE_BUILD_RPATH_USE_ORIGIN TRUE)
endif()

target_link_libraries(${TARGET} PRIVATE Threads::Threads GTest::gtest_main GTest::gmock)
target_link_libraries(${TARGET} PRIVATE AdsBridge)