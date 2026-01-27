
project(AdsGatewayLib LANGUAGES CXX)

set(TARGET AdsGatewayLib)

add_library(${TARGET} STATIC
  ./include/AdsGatewayLib/AdsGwConnection.hpp
  ./include/AdsGatewayLib/AdsGwProtocol.hpp
  ./include/AdsGatewayLib/GwCrc32.hpp
  ./include/AdsGatewayLib/GwHash.hpp
  ./include/AdsGatewayLib/ISignalUpdater.hpp
  ./include/AdsGatewayLib/Logger.hpp
  ./include/AdsGatewayLib/SignalManager.hpp

  ./src/AdsGwConnection.cpp
  ./src/AdsGwConnImpl.cpp
  ./src/AdsGwConnImpl.hpp
  ./src/GwCrc32.cpp
  ./src/SignalManager.cpp
  ./src/TcpConnection.cpp
  ./src/TcpConnection.hpp
  ./src/TcpConnection.hpp
)

target_compile_features(${TARGET} PRIVATE cxx_std_20)

if (WIN32)
    target_sources(${TARGET} PRIVATE
        ./src/TcpConnWindows.cpp
        ./src/TcpConnWindows.hpp
    )
else()
    target_sources(${TARGET} PRIVATE
        ./src/TcpConnLinux.cpp
        ./src/TcpConnLinux.hpp
    )
endif()

target_compile_features(${TARGET} PRIVATE cxx_std_20) 

if (WIN32)
    target_link_libraries(${TARGET} PRIVATE Ws2_32)
endif()

target_include_directories(${TARGET} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(${TARGET} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
