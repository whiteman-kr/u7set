@echo off

echo Compile Protobuf file serialization.proto
call ..\Protobuf\protoc.exe --cpp_out=. serialization.proto
move /Y serialization.pb.cc body.pb.cc
copy warningguardstart.cc+body.pb.cc+warningguardend.cc /a serialization.pb.cc /b
del body.pb.cc

echo Compile Protobuf file network.proto 
call ..\Protobuf\protoc.exe --cpp_out=. network.proto
move /Y network.pb.cc body.pb.cc
copy warningguardstart.cc+body.pb.cc+warningguardend.cc /a network.pb.cc /b
del body.pb.cc

