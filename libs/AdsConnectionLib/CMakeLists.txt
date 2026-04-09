project(AdsConnectionLib LANGUAGES CXX)

set(TARGET AdsConnectionLib)

add_library(${TARGET} STATIC
	./include/AdsConnectionLib/AdsConnection.h
	./include/AdsConnectionLib/ClientConnStatsStd.h
	./include/AdsConnectionLib/ClientGrpc.h
	./include/AdsConnectionLib/ILoggerStd.h
	./include/AdsConnectionLib/ISignalLogUpdater.h
	./include/AdsConnectionLib/ServiceConnectionState.h
	./include/AdsConnectionLib/ServiceEndpoint.h
		
	./src/AdsConnection.cpp
	./src/AdsConnectionPrivate2.cpp
	./src/AdsConnectionPrivate2.h
	./src/ClientConnStatsStd.cpp
	./src/ClientGrpc.cpp
)

target_precompile_headers(${TARGET} PRIVATE
	<CommonStdLib/HashStd.h>

	<Network.pb.h>
	<GrpcAppDataSrv.grpc.pb.h>
)

target_compile_definitions(${TARGET} PRIVATE USE_GRPC_ADS_CONNECTION)

set_target_properties(${TARGET} PROPERTIES POSITION_INDEPENDENT_CODE ON) # Ensure for shared libraries (AdsBridge)

target_link_libraries(${TARGET} PUBLIC CommonStdLib AppSignalLibStd GrpcLib)

target_include_directories(${TARGET} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(${TARGET} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(${TARGET} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_code_coverage_options(${TARGET} CODE_COVERAGE)
