cmake_minimum_required(VERSION 3.16)

project(UiLib LANGUAGES CXX)

include(../../cmake/translations.cmake)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets LinguistTools)

set(TARGET UiLib)

qt_add_library(${TARGET} STATIC
	./include/UiLib/CodeEditor.h
	CodeEditor.cpp
)

target_precompile_headers(${TARGET} PRIVATE
#	<algorithm>

	<QApplication>
	<QPainter>
	<QRegularExpression>
	<QTextDocumentFragment>
	<QTimer>
)

target_precompile_headers(${TARGET} PRIVATE
	"../../CommonLib/Types.h"
	"../../CommonLib/Hash.h"
)

target_compile_definitions(${TARGET} PRIVATE UI_LIB_DOMAIN)

target_link_libraries(${TARGET} PUBLIC Qt6::Widgets)
target_link_libraries(${TARGET} PUBLIC CommonLib UtilsLib)

target_include_directories(${TARGET} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(${TARGET} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(${TARGET} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_code_coverage_options(${TARGET} CODE_COVERAGE)
