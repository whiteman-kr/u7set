cmake_minimum_required(VERSION 3.16)

project(CommonLib LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
find_package(Qt6 REQUIRED COMPONENTS Core)

set(TARGET CommonLib)

qt_add_library(${TARGET} STATIC
  AfbParamValue.h
  AfbParamValue.cpp
  DebugInstCounter.h
  Factory.h
  Hash.h
  HashedVector.h
  HostAddressPort.h
  PropertyObject.cpp
  PropertyObject.h
  Times.cpp
  Times.h
  Types.cpp
  Types.h
)

#target_precompile_headers(${TARGET} PUBLIC
#	"Types.h"
#	"Hash.h"
#	"PropertyObject.h"
#	"AfbParamValue.h"
#)

target_link_libraries(${TARGET} PUBLIC Qt6::Core)
target_compile_definitions(${TARGET} PRIVATE COMMON_LIB_DOMAIN)
target_include_directories(${TARGET} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

add_code_coverage_options(${TARGET} CODE_COVERAGE)
