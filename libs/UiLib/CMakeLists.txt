cmake_minimum_required(VERSION 3.16)

project(UiLib LANGUAGES CXX)

include(../../cmake/translations.cmake)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets LinguistTools PrintSupport)

set(TARGET UiLib)

qt_add_library(${TARGET} STATIC
	./include/UiLib/ChooseItemsWidget.h
	./include/UiLib/ClickableLabel.h
	./include/UiLib/CodeEditor.h
	./include/UiLib/DialogAbout.h
	./include/UiLib/DialogAlert.h
	./include/UiLib/DialogProgress.h
	./include/UiLib/DialogProgress.h
	./include/UiLib/DoubleValidatorEx.h
	./include/UiLib/ExportPrint.h
	./include/UiLib/PropertyEditor.h
	./include/UiLib/PropertyEditorDialog.h
	./include/UiLib/StandardColors.h
	./include/UiLib/TabWidgetEx.h
	./include/UiLib/TagSelectorWidget.h
	ChooseItemsWidget.cpp
	CodeEditor.cpp
	DialogAbout.cpp
	DialogAlert.cpp
	DialogProgress.cpp
	DialogProgress.cpp
	ExportPrint.cpp
	PropertyEditor.cpp
	PropertyEditorDialog.cpp
	TabWidgetEx.cpp
	TagSelectorWidget.cpp
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

	"../../CommonLib/PropertyObject.h"
	"../../CommonLib/Types.h"
	"../../CommonLib/Hash.h"
)

target_compile_definitions(${TARGET} PRIVATE UI_LIB_DOMAIN)

target_link_libraries(${TARGET} PUBLIC Qt6::Widgets Qt6::PrintSupport)
target_link_libraries(${TARGET} PUBLIC CommonLib UtilsLib Protobuf AppSignalLib)

target_include_directories(${TARGET} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(${TARGET} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(${TARGET} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_code_coverage_options(${TARGET} CODE_COVERAGE)
