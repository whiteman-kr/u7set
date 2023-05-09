cmake_minimum_required(VERSION 3.16)

project(CommonLib LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

#find_package(QT NAMES Qt6 REQUIRED COMPONENTS Core Qml)
find_package(Qt6 REQUIRED COMPONENTS Core)

add_library(CommonLib STATIC
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

target_link_libraries(CommonLib PRIVATE Qt6::Core)
target_compile_definitions(CommonLib PRIVATE COMMONLIB_LIBRARY)
target_include_directories(CommonLib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

add_compile_definitions(COMMON_LIB_DOMAIN)		# TODO: COMMON_LIB_DOMAIN -> COMMONLIB_LIBRARY?


