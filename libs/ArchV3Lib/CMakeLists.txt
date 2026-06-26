cmake_minimum_required(VERSION 3.16)

project(ArchV3Lib LANGUAGES CXX)

#set(CMAKE_AUTOUIC ON)
#set(CMAKE_AUTOMOC ON)
#set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Sql)

set(TARGET ArchV3Lib)

qt_add_library(${TARGET} STATIC
	./include/ArchV3Lib/ArchFile.h
	./include/ArchV3Lib/Core.h
	./include/ArchV3Lib/Db.h
	./include/ArchV3Lib/Postgres.h
	./include/ArchV3Lib/Utils.h
	./include/ArchV3Lib/Writer.h
	ArchFile.cpp
	Core.cpp
	Db.cpp
	Writer.cpp
	Postgres.cpp

	ArchV3Lib.qrc
)

target_precompile_headers(${TARGET} PRIVATE
	# C++ includes
	#
	<algorithm>
	<atomic>
	<cassert>
	<cmath>
	<condition_variable>
	<cstdint>
	<cstdlib>
	<functional>
	<limits>
	<list>
	<map>
	<memory>
	<optional>
	<ranges>
	<queue>
	<set>
	<type_traits>
	<unordered_map>
	<utility>
	<vector>

	# Qt includes
	#
	<QDate>
	<QDebug>
	<QFile>
	<QFileInfo>
	<QMetaObject>
	<QMetaProperty>
	<QObject>
	<QTime>
	<QUuid>
)

target_precompile_headers(${TARGET} PRIVATE
	<CommonLib/Types.h>
	<CommonLib/Hash.h>
	<Network.pb.h>
)

target_compile_definitions(${TARGET} PRIVATE ARCH_V3_LIB_DOMAIN)

target_link_libraries(${TARGET} PUBLIC Qt6::Core Qt6::Sql)
target_link_libraries(${TARGET} PUBLIC CommonLib ProtobufEx UtilsLib AppSignalLib)

target_include_directories(${TARGET} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(${TARGET} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(${TARGET} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_code_coverage_options(${TARGET} CODE_COVERAGE)
