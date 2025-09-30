cmake_minimum_required(VERSION 3.16)

project(LicenserBlacklistGenerator LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

qt_standard_project_setup()

set(TARGET LicenserBlacklistGenerator)

add_executable(${TARGET}
	./src/main.cpp
)

 target_precompile_headers(${TARGET} PRIVATE
	<CommonLib/Types.h>
	<CommonLib/PropertyObject.h>
)

target_link_libraries(${TARGET} LicenseLib)
