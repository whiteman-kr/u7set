cmake_minimum_required(VERSION 3.25)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(VDUFontGenerator LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

if (MSVC)
  # MSVC warning suppressions
  add_compile_options(
	/wd4828 # The file contains a character starting at offset 0x10d2 that is illegal in the current source character set (codepage 65001).
  )
endif (MSVC)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Network Qml Xml Svg PrintSupport LinguistTools)
qt_standard_project_setup()

set (VDUFONT_SOURCES
	VDUFontGenerator.cpp
	VDUFontGenerator.h
	main.cpp
)

set(TARGET VDUFontGenerator)

add_executable(${TARGET} WIN32
	${VDUFONT_SOURCES}
)

target_precompile_headers(${TARGET} PRIVATE
	# C includes, must be before c++ includes
	#
	<cassert>
	<cstdint>

	# C++ includes
	#
	<algorithm>
	<array>
	<atomic>
	<chrono>
	<concepts>
	<deque>
	<functional>
	<limits>
	<list>
	<map>
	<memory>
	<queue>
	<ranges>
	<set>
	<type_traits>
	<vector>

	# Qt includes
	#
	<QtPrintSupport>
	<QtQml>
	<QtWidgets>
	<QtXml>

	# Project includes
	#
)

target_link_libraries(${TARGET} PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Network Qt6::Qml Qt6::Xml Qt6::Svg Qt6::PrintSupport)


