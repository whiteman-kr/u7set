cmake_minimum_required(VERSION 3.28)

project(Licenser LANGUAGES CXX)

include(../../cmake/vld.cmake)
include(../../cmake/translations.cmake)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Sql Network Xml Widgets Gui)
qt_standard_project_setup()

set(TARGET Licenser)

add_executable(${TARGET} WIN32
	./src/LicenserMainWindow.cpp
	./src/LicenserMainWindow.h
	./src/main.cpp
	./src/PropertyEditorWithUpdate.cpp
	./src/PropertyEditorWithUpdate.h
	./src/RpctLicenseObject.cpp
	./src/RpctLicenseObject.h

	../../Etc/Logo/RadiyLogo.qrc
)

 target_precompile_headers(${TARGET} PRIVATE
	<CommonLib/Types.h>
	<CommonLib/PropertyObject.h>
)

target_link_libraries(${TARGET} Qt6::Gui Qt6::Widgets)
target_link_libraries(${TARGET} CommonLib UiLib LicenseLib)
