cmake_minimum_required(VERSION 3.16)

project(AppSignalLists LANGUAGES CXX)

include(../../cmake/translations.cmake)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Gui Xml Network Widgets LinguistTools Qml)

set(TARGET AppSignalLists)

qt_add_library(${TARGET} STATIC
	./include/AppSignalLists/SignalList.h
	./include/AppSignalLists/SignalListEditor.h
	SignalList.cpp
	SignalListEditor.cpp
)

target_precompile_headers(${TARGET} PRIVATE
	<algorithm>
	<cassert>
	<chrono>
	<cstdint>
	<functional>
	<list>
	<map>
	<memory>
	<mutex>
	<queue>
	<ranges>
	<set>
	<unordered_map>
	<unordered_set>
	<vector>

	<QApplication>
	<QComboBox>
	<QCoreApplication>
	<QDateTime>
	<QDialog>
	<QDomDocument>
	<QHBoxLayout>
	<QJSEngine>
	<QLabel>
	<QLineEdit>
	<QLocale>
	<QMessageBox>
	<QThread>
	<QPushButton>
	<QReadWriteLock>
	<QSharedMemory>
	<QThread>
	<QTimeZone>
	<QTranslator>
	<QVBoxLayout>
	<QXmlStreamReader>
	<QXmlStreamWriter>
	<QTreeWidget>
	<QTableView>
	<QSplitter>
	<QCheckBox>
	<QHeaderView>
	<QMenu>
	<QFile>
	<QFileInfo>
	<QFileDialog>
	<QDialogButtonBox>
	<QSettings>

	"../../CommonLib/Types.h"
	"../../CommonLib/Hash.h"
	"../../CommonLib/PropertyObject.h"

	"../../Proto/ProtoSerialization.h"
	"../../Proto/ProtoCommonHelper.h"
	"../../Proto/AppSignalList.pb.h"

	"../../AppSignalLib/AppSignalParam.h"
	"../../AppSignalLib/AppSignalState.h"
	"../../AppSignalLib/TuningValue.h"

)

target_compile_definitions(${TARGET} PRIVATE APPSIGNALLISTS_LIB_DOMAIN)

#Add translations
#
add_translation(${TARGET} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

target_link_libraries(${TARGET} PUBLIC Qt6::Gui Qt6::Xml Qt6::Network Qt6::Widgets Qt6::Qml)
target_link_libraries(${TARGET} PUBLIC CommonLib Protobuf UtilsLib OnlineLib AppSignalLib TrendView)

#if (MSVC)
	#target_link_libraries(${TARGET} PUBLIC Advapi32)
#endif (MSVC)
#
#if (UNIX)
	#target_link_libraries(${TARGET} PUBLIC pam pam_misc)
#endif (UNIX)

target_include_directories(${TARGET} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(${TARGET} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(${TARGET} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_code_coverage_options(${TARGET} CODE_COVERAGE)
