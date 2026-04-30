cmake_minimum_required(VERSION 3.16)

# These libraries are not dependend on Qt libraries
#
add_subdirectory(AdsBridge)
add_subdirectory(AdsConnectionLib)
add_subdirectory(AppSignalLibStd)
add_subdirectory(CommonStdLib)
add_subdirectory(GrpcLib)
add_subdirectory(Protos)

if (NOT STATIC_RUNTIME)
	# These libraries require dynamic runtime
	# They are dependend on Qt libraries
	#
	add_subdirectory(AppSignalLists)
	add_subdirectory(Behavior)
	add_subdirectory(BuildCompLib)
	add_subdirectory(ClientLib)
	add_subdirectory(CommonLib)
	add_subdirectory(DbLib)
	add_subdirectory(DiagClientLib)
	add_subdirectory(DiagStateLib)
	add_subdirectory(GatewayClientLib)
	add_subdirectory(HardwareLib)
	add_subdirectory(LicenseLib)
	add_subdirectory(ModuleConfiguratorLib)
	add_subdirectory(ProtobufEx)
	add_subdirectory(ReportLib)
	add_subdirectory(SchemaClientLib)
	add_subdirectory(ServiceLib)
	add_subdirectory(TestSuiteLib)
	add_subdirectory(TrendView)
	add_subdirectory(TuningLib)
	add_subdirectory(UiLib)
	add_subdirectory(VFrame30)
endif()