@echo off

echo Compile Protobuf file serialization.proto
call ..\Protobuf\protoc.exe --cpp_out=. serialization.proto
if NOT %ERRORLEVEL% == 0 goto :reporterror

move /Y serialization.pb.cc body.pb.cc
copy warningguardstart.cc+body.pb.cc+warningguardend.cc /a serialization.pb.cc /b
del body.pb.cc
move /Y serialization.pb.h body.pb.h
copy warningguardstart.cc+body.pb.h+warningguardend.cc /a serialization.pb.h /b
del body.pb.h

echo Compile Protobuf file network.proto 
call ..\Protobuf\protoc.exe --cpp_out=. network.proto
if NOT %ERRORLEVEL% == 0 goto :reporterror
move /Y network.pb.cc body.pb.cc
copy warningguardstart.cc+body.pb.cc+warningguardend.cc /a network.pb.cc /b
del body.pb.cc
move /Y network.pb.h body.pb.h
copy warningguardstart.cc+body.pb.h+warningguardend.cc /a network.pb.h /b
del body.pb.h

echo Compile Protobuf file trends.proto 
call ..\Protobuf\protoc.exe --cpp_out=. trends.proto
if NOT %ERRORLEVEL% == 0 goto :reporterror
move /Y trends.pb.cc body.pb.cc
copy warningguardstart.cc+body.pb.cc+warningguardend.cc /a trends.pb.cc /b
del body.pb.cc
move /Y trends.pb.h body.pb.h
copy warningguardstart.cc+body.pb.h+warningguardend.cc /a trends.pb.h /b
del body.pb.h
                                      
goto :endoffile

:reporterror
echo@ 
echo@ Error compilation Proto file
goto :endoffile

:endoffile

