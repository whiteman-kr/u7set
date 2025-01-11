cmake_minimum_required(VERSION 3.28)

project(TuningLib LANGUAGES CXX)

include(../../cmake/translations.cmake)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets LinguistTools PrintSupport Xml)

set(TARGET TuningLib)

qt_add_library(${TARGET} STATIC
	./include/TuningLib/TuningFilter.h
	./include/TuningLib/TuningFilterToLists.h
	./include/TuningLib/TuningUiConstants.h
	./include/TuningLib/TuningUiItem.h
	TuningFilter.cpp
	TuningFilterToLists.cpp
	TuningUiItem.cpp
)

target_precompile_headers(${TARGET} PRIVATE
	<algorithm>
	<map>
	<set>
	<vector>

	<QAbstractItemModel>
	<QApplication>
	<QClipboard>
	<QColorDialog>
	<QComboBox>
	<QDialog>
	<QDialogButtonBox>
	<QFile>
	<QFileDialog>
	<QFontDialog>
	<QHBoxLayout>
	<QHeaderView>
	<QLabel>
	<QLineEdit>
	<QMenu>
	<QMessageBox>
	<QMouseEvent>
	<QPageSetupDialog>
	<QPainter>
	<QPlainTextEdit>
	<QPrintDialog>
	<QPrinter>
	<QProgressDialog>
	<QPushButton>
	<QSettings>
	<QSplitter>
	<QTableView>
	<QTabWidget>
	<QTextBrowser>
	<QTextCursor>
	<QTextDocument>
	<QTextDocumentFragment>
	<QTextDocumentWriter>
	<QTimer>
	<QToolButton>
	<QTreeWidget>
	<QTreeWidgetItem>
	<QVBoxLayout>
	<QXmlStreamReader>
	<QXmlStreamWriter>

	<CommonLib/PropertyObject.h>
	<CommonLib/Types.h>
	<CommonLib/Hash.h>
)

target_compile_definitions(${TARGET} PRIVATE UI_LIB_DOMAIN)

target_link_libraries(${TARGET} PUBLIC Qt6::Widgets Qt6::PrintSupport Qt6::Xml)
target_link_libraries(${TARGET} PUBLIC CommonLib UtilsLib ProtobufEx AppSignalLib AppSignalLists)

target_include_directories(${TARGET} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(${TARGET} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(${TARGET} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_code_coverage_options(${TARGET} CODE_COVERAGE)
