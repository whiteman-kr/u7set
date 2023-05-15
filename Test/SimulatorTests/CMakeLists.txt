cmake_minimum_required(VERSION 3.16)

project(SimulatorTests LANGUAGES CXX)
message("${PROJECT_NAME}, Version: ${CMAKE_PROJECT_VERSION}")

enable_testing()

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt6 REQUIRED COMPONENTS Test Core Network Qml Xml Concurrent)
qt_standard_project_setup()

set(TARGET_NAME SimulatorTests)

add_executable(${TARGET_NAME}
	main.cpp
	SimAfbParamTests.cpp
	SimAfbParamTests.h
	SimCommandTest_LM5_LM6.cpp
	SimCommandTest_LM5_LM6.h
	SimProfilesTest.cpp
	SimProfilesTest.h
	SimRamTests.cpp
	SimRamTests.h

	Resources.qrc
)

set_target_properties(${TARGET_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

target_link_libraries(${TARGET_NAME} PRIVATE Qt6::Test Qt6::Core Qt6::Network Qt6::Qml Qt6::Xml Qt6::Concurrent)
target_link_libraries(${TARGET_NAME} PRIVATE Protobuf HardwareLib Simulator)


