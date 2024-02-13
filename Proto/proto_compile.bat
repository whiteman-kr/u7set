rem @echo off

call ..\Protobuf\protoc.exe --cpp_out=. BuildInfo.proto
call ..\Protobuf\protoc.exe --cpp_out=. Common.proto
call ..\Protobuf\protoc.exe --cpp_out=. ArchSignal.proto
call ..\Protobuf\protoc.exe --cpp_out=. AppSignal.proto
call ..\Protobuf\protoc.exe --cpp_out=. Comparator.proto
call ..\Protobuf\protoc.exe --cpp_out=. Simulator.proto
call ..\Protobuf\protoc.exe --cpp_out=. Serialization.proto
call ..\Protobuf\protoc.exe --cpp_out=. Network.proto
call ..\Protobuf\protoc.exe --cpp_out=. Trends.proto

goto :endoffile

rem 
rem echo Compile Protobuf file Serialization.proto
rem call ..\Protobuf\protoc.exe --cpp_out=. Serialization.proto
rem if NOT %ERRORLEVEL% == 0 goto :reporterror
rem 
rem move /Y serialization.pb.cc body.pb.cc
rem copy warningguardstart.cc+body.pb.cc+warningguardend.cc /a serialization.pb.cc /b
rem del body.pb.cc
rem move /Y serialization.pb.h body.pb.h
rem copy warningguardstart.cc+body.pb.h+warningguardend.cc /a serialization.pb.h /b
rem del body.pb.h

rem echo Compile Protobuf file Network.proto 
rem call ..\Protobuf\protoc.exe --cpp_out=. Network.proto
rem if NOT %ERRORLEVEL% == 0 goto :reporterror
rem move /Y network.pb.cc body.pb.cc
rem copy warningguardstart.cc+body.pb.cc+warningguardend.cc /a network.pb.cc /b
rem del body.pb.cc
rem move /Y network.pb.h body.pb.h
rem copy warningguardstart.cc+body.pb.h+warningguardend.cc /a network.pb.h /b
rem del body.pb.h

rem echo Compile Protobuf file Trends.proto 
rem call ..\Protobuf\protoc.exe --cpp_out=. Trends.proto
rem if NOT %ERRORLEVEL% == 0 goto :reporterror
rem move /Y trends.pb.cc body.pb.cc
rem copy warningguardstart.cc+body.pb.cc+warningguardend.cc /a trends.pb.cc /b
rem del body.pb.cc
rem move /Y trends.pb.h body.pb.h
rem copy warningguardstart.cc+body.pb.h+warningguardend.cc /a trends.pb.h /b
rem del body.pb.h
                                      
rem goto :endoffile

rem :reporterror
rem echo 
rem echo Error compilation Proto file
rem goto :endoffile

:endoffile

