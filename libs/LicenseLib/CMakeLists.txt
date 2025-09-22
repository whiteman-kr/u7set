cmake_minimum_required(VERSION 3.16)

project(LicenseLib LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(OpenSSL REQUIRED)

set(TARGET LicenseLib)

qt_add_library(${TARGET} STATIC
	./include/LicenseLib/AppLicenser.h
	./include/LicenseLib/RpctLicense.h
	./include/LicenseLib/RpctValidator.h
	./src/AppLicenser.cpp
	./src/Blacklist.cpp
	./src/Blacklist.h
	./src/BlacklistItem.h
	./src/RpctLicense.cpp
	./src/RpctValidator.cpp

	./src/LicenseLib.qrc
)

target_precompile_headers(${TARGET} PRIVATE
	"./src/BlacklistItem.h"

	<CommonLib/Types.h>
	<CommonLib/Hash.h>
)

target_link_libraries(${TARGET} PUBLIC Qt6::Widgets Qt6::Network OpenSSL::SSL OpenSSL::Crypto)
target_link_libraries(${TARGET} PUBLIC CommonLib ProtobufEx)

target_include_directories(${TARGET} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(${TARGET} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Create license directory
#
add_custom_command(
	TARGET ${TARGET}
	POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/license
)

add_custom_command(
	TARGET ${TARGET}
	POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_LIST_DIR}/readme.txt ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/license
)

add_code_coverage_options(${TARGET} CODE_COVERAGE)

if(BUILD_SMALL_TESTS)
	add_subdirectory(tests)
endif()
