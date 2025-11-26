
project(CommonStdLib LANGUAGES CXX)

set(TARGET CommonStdLib)

add_library(${TARGET} STATIC
  ./include/CommonStdLib/HashStd.h
  ./include/CommonStdLib/TimesStd.h
  ./include/CommonStdLib/u7_vld.h
  
  ./src/u7_vld.cpp
)

set_target_properties(${TARGET} PROPERTIES POSITION_INDEPENDENT_CODE ON) # Ensure for shared libraries (AdsBridge)

target_compile_definitions(${TARGET} PRIVATE COMMON_STD_LIB_DOMAIN)
target_include_directories(${TARGET} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(${TARGET} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Link Visaul Leak Detector
#
if(CMAKE_BUILD_TYPE STREQUAL "Debug" AND MSVC)
	
	set(VLD_PROJECT_ROOT_PATH "${CMAKE_SOURCE_DIR}")
	string(REPLACE "/" "\\\\"
	    VLD_PROJECT_ROOT_PATH_ESCAPED "${VLD_PROJECT_ROOT_PATH}")

	add_compile_definitions(VLD_PROJECT_ROOT_PATH=L"${VLD_PROJECT_ROOT_PATH_ESCAPED}")

	target_link_libraries(${TARGET} PUBLIC "C:/Program Files (x86)/Visual Leak Detector/lib/Win64/vld.lib")
endif()

add_code_coverage_options(${TARGET} CODE_COVERAGE)
