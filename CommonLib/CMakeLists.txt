cmake_minimum_required(VERSION 3.16)

project(CommonLib LANGUAGES CXX)
message("${PROJECT_NAME}, Version: ${CMAKE_PROJECT_VERSION}")

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
find_package(Qt6 REQUIRED COMPONENTS Core)

set(TARGET CommonLib)

add_library(${TARGET} STATIC
  AfbParamValue.h
  AfbParamValue.cpp
  DebugInstCounter.h
  Factory.h
  Hash.h
  HostAddressPort.h
  OrderedHash.h
  PropertyObject.cpp
  PropertyObject.h
  Times.cpp
  Times.h
  Types.cpp
  Types.h
)

target_link_libraries(${TARGET} PUBLIC Qt6::Core)
target_compile_definitions(${TARGET} PRIVATE COMMON_LIB_DOMAIN)
target_include_directories(${TARGET} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

add_code_coverage_options(${TARGET} CODE_COVERAGE)