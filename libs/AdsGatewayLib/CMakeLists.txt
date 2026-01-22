
project(AdsGatewayLib LANGUAGES CXX)

set(TARGET AdsGatewayLib)

add_library(${TARGET} STATIC
  ./include/AdsGatewayLib/AdsGwProtocol.hpp
  ./include/AdsGatewayLib/GwCrc32.hpp
  ./include/AdsGatewayLib/GwHash.hpp
  
  ./src/GwCrc32.cpp
)

target_compile_features(${TARGET} PRIVATE cxx_std_20)

target_include_directories(${TARGET} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(${TARGET} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
