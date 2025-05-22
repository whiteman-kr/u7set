cmake_minimum_required(VERSION 3.16)

project(SimServiceClientLib LANGUAGES CXX)

set(TARGET SimServiceClientLib)

qt_add_library(${TARGET} STATIC
	./include/SimServiceClientLib/SimServiceClient.h
	./include/SimServiceClientLib/SimServiceModule.h
	
	./src/SimServiceClient.cpp
	./src/SimServiceClientImpl.cpp
	./src/SimServiceClientImpl.h
	./src/SimServiceModule.cpp
	
)

target_precompile_headers(${TARGET} PUBLIC
	<CommonLib/Types.h>
	<CommonLib/Hash.h>

	<AppSignal.pb.h>
	<SimService.pb.h>
	<SimService.grpc.pb.h>
	
	"../../AppSignalLib/AppSignal.h"
	"../../AppSignalLib/AppSignalParam.h"
	"../../AppSignalLib/AppSignalState.h"
	"../../AppSignalLib/TuningValue.h"
)

target_link_libraries(${TARGET} PUBLIC Qt6::Core)
target_link_libraries(${TARGET} PUBLIC CommonLib AppSignalLib SimulatorGrpc)

target_include_directories(${TARGET} PRIVATE ${CMAKE_SOURCE_DIR})
target_include_directories(${TARGET} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(${TARGET} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)


add_code_coverage_options(${TARGET} CODE_COVERAGE)

add_subdirectory(example)