cmake_minimum_required(VERSION 3.21)

project (AdsGatewayExample)

set(TARGET AdsGatewayExample)

add_executable(${TARGET} 
    ./src/main.cpp
    ./src/AdsGwConnection.cpp
    ./src/AdsGwConnection.hpp
    ./src/TcpConnection.cpp
    ./src/TcpConnection.hpp
)

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

target_link_libraries(${TARGET} PRIVATE AdsGatewayLib)

