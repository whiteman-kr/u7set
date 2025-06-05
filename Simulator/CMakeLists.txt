#
# Simulator related projects
#

add_subdirectory(SimServiceClientLib)
add_subdirectory(SimulatorConsole)
add_subdirectory(SimulatorLib)
add_subdirectory(SimulatorTests)

# Add the SimulatorUi project only if code coverage is not enabled
# mosth likely this is a test build, so we don't need the UI
#
if(NOT CODE_COVERAGE)
	add_subdirectory(Simulator)
	add_subdirectory(SimulatorUi)
	add_subdirectory(RvModelSimBridge)
	add_subdirectory(RvModelSimTester)
endif()
