project(AppSignalLibStd LANGUAGES CXX)

set(TARGET AppSignalLibStd)

add_library(${TARGET} STATIC
	./include/AppSignalLibStd/AppSignalAccessor.h
	./include/AppSignalLibStd/AppSignalManagerCore.h
	./include/AppSignalLibStd/IAppSignalUpdater.h
	./include/AppSignalLibStd/IRecentAppSignals.h
	./include/AppSignalLibStd/ISignalDataServer.h
	./include/AppSignalLibStd/ISignalManagerT.h
	./include/AppSignalLibStd/RecentUsed.h
	
	./src/AppSignalManagerCore.cpp
	./src/RecentUsed.cpp
)

target_precompile_headers(${TARGET} PRIVATE
	<chrono>

	<CommonStdLib/HashStd.h>
	<Network.pb.h>
)

target_compile_definitions(${TARGET} PRIVATE USE_GRPC_ADS_CONNECTION)

set_target_properties(${TARGET} PROPERTIES POSITION_INDEPENDENT_CODE ON) # Ensure for shared libraries (AdsBridge)

target_link_libraries(${TARGET} PUBLIC CommonStdLib Protos)

target_include_directories(${TARGET} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(${TARGET} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(${TARGET} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_code_coverage_options(${TARGET} CODE_COVERAGE)
