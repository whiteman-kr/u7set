cmake_minimum_required(VERSION 3.16)

project(Archivist LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)

add_executable(Archivist
	../ArchivingService/ArchFilePartition.cpp
	../ArchivingService/ArchFileRecord.cpp
	Archivist.cpp
	Archivist.h
	ArchivistMain.cpp
	DbArchivist.cpp
	DbArchivist.h
	FileArchivist.cpp
	FileArchivist.h
	Print.cpp
	Print.h
)

target_precompile_headers(Archivist PRIVATE
	# C includes, must be before c++ includes
	#
	<cassert>
	<cstdint>

	# C++ includes
	#
	<atomic>
	<iostream>
	<limits>
	<list>
	<map>
	<memory>
	<vector>
	<set>
	<type_traits>

	# Qt includes
	#
	<QCoreApplication>
	<QDebug>
	<QDate>
	<QDateTime>
	<QFile>
	<QFileInfo>
	<QObject>
	<QString>
	<QStringList>
	<QStorageInfo>
	<QTextStream>
	<QThread>
	<QtGlobal>

	# Project includes
	#
	<CommonLib/Types.h>
	<CommonLib/Hash.h>

	<Network.pb.h>

	<ArchSignal.pb.h>

	"../AppSignalLib/AppSignal.h"
)


target_link_libraries(Archivist Qt${QT_VERSION_MAJOR}::Core CommonLib ProtobufEx UtilsLib AppSignalLib)
#target_link_libraries(Archivist PRIVATE CommonLib ProtobufEx UtilsLib AppSignalLib)

include(GNUInstallDirs)
install(TARGETS Archivist
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
