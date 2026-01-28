cmake_minimum_required(VERSION 3.21)

project (AdsGatewayExample LANGUAGES CXX)

set(TARGET AdsGatewayExample)

add_executable(${TARGET} 
    ./src/main.cpp
)

target_link_libraries(${TARGET} PRIVATE AdsGatewayLib)
