cmake_minimum_required(VERSION 3.16)

project(DbLib LANGUAGES CXX)
message("${PROJECT_NAME}, Version: ${CMAKE_PROJECT_VERSION}")

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Gui Widgets Sql Concurrent Network)

add_library(DbLib STATIC
	DbController.cpp
	DbController.h
	DbControllerTools.cpp
	DbControllerTools.h
	DbObjectStorage.h
	DbProgress.cpp
	DbProgress.h
	DbProgressDialog.cpp
	DbProgressDialog.h
	DbStruct.cpp
	DbStruct.h
	DbWorker.cpp
	DbWorker.h
)

target_link_libraries(DbLib PUBLIC Qt6::Gui Qt6::Widgets Qt6::Sql Qt6::Concurrent Qt6::Network)
target_link_libraries(DbLib PUBLIC CommonLib HardwareLib AppSignalLib)

target_compile_definitions(DbLib PRIVATE DB_LIB_DOMAIN)
#target_include_directories(DbLib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

