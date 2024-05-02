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
	SignalListEditorPrivate.cpp
	SignalListEditorPrivate.h
	TextResource.h
)

target_precompile_headers(${TARGET} PRIVATE
	<algorithm>
	<cassert>
	<cstdint>
	<list>
	<map>
	<memory>
	<ranges>
	<set>
	<vector>

	<QApplication>
	<QCheckBox>
	<QComboBox>
	<QCoreApplication>
	<QDateTime>
	<QDialog>
	<QDialogButtonBox>
	<QFile>
	<QFileDialog>
	<QFileInfo>
	<QHBoxLayout>
	<QHeaderView>
	<QLabel>
	<QLineEdit>
	<QMenu>
	<QMessageBox>
	<QPushButton>
	<QReadWriteLock>
	<QSettings>
	<QSplitter>
	<QTableView>
	<QTranslator>
	<QTreeWidget>
	<QVBoxLayout>

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

target_link_libraries(${TARGET} PUBLIC Qt6::Gui Qt6::Widgets)
target_link_libraries(${TARGET} PUBLIC CommonLib Protobuf UtilsLib AppSignalLib)

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
