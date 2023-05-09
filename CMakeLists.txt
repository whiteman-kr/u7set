cmake_minimum_required(VERSION 3.16)

project(u7)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
	set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

add_subdirectory(Protobuf)
add_subdirectory(CommonLib)
add_subdirectory(UtilsLib)
add_subdirectory(HardwareLib)
add_subdirectory(AppSignalLib)
add_subdirectory(OnlineLib)
add_subdirectory(DbLib)
add_subdirectory(ServiceLib)
add_subdirectory(ClientLib)

add_subdirectory(TrendView)
add_subdirectory(VFrame30)

add_subdirectory(Simulator)
add_subdirectory(SimulatorConsole)

add_subdirectory(Builder)
add_subdirectory(BuilderConsole)

add_subdirectory(./Tools/qtkeychain)
add_subdirectory(u7)

add_subdirectory(ServiceControlManager)
add_subdirectory(ConfigurationService)
add_subdirectory(AppDataService)
add_subdirectory(ArchivingService)
add_subdirectory(TuningService)
add_subdirectory(DiagDataService)

add_subdirectory(Monitor)
add_subdirectory(TuningClient)

add_subdirectory(Metrology)
add_subdirectory(mconf)

add_subdirectory(TestSuiteLib)
add_subdirectory(TestSuite)
add_subdirectory(TestSuiteConsole)

add_subdirectory(./Tools/PacketSource)
add_subdirectory(./Tools/PacketViewer)

add_subdirectory(./Test/googletest)
add_subdirectory(./Test/UalTester)
add_subdirectory(./Test/ClientTests)
add_subdirectory(./Test/SimulatorTests)
add_subdirectory(./Test/u7databaseTests)
add_subdirectory(./Test/MetrologyTests)
#add_subdirectory(./Test/TestSuiteTest)



