@echo off

setlocal enabledelayedexpansion

for %%f in (*.proto) do (
    echo --- Compile Protobuf file %%f ---
    echo Processing file: %%f
    echo Filename without extension: %%~nf
    set "fileName=%%~nf"
    echo Filename variable: !fileName!

    call ..\Protobuf\protoc.exe --cpp_out=. "%%f"
    if NOT !ERRORLEVEL! == 0 goto :reporterror

    move /Y !fileName!.pb.cc body.pb.cc
    copy warningguardstart.cc+body.pb.cc+warningguardend.cc /a !fileName!.pb.cc /b
    del body.pb.cc

    move /Y !fileName!.pb.h body.pb.h
    copy warningguardstart.cc+body.pb.h+warningguardend.cc /a !fileName!.pb.h /b
    del body.pb.h
)
goto :endoffile

:reporterror
echo 
echo Error compilation Proto file
goto :endoffile

:endoffile

rem echo Compile Protobuf file Serialization.proto
rem call ..\Protobuf\protoc.exe --cpp_out=. Serialization.proto
rem if NOT %ERRORLEVEL% == 0 goto :reporterror

rem move /Y serialization.pb.cc body.pb.cc
rem warningguardstart.cc+body.pb.cc+warningguardend.cc /a serialization.pb.cc /b
rem del body.pb.cc
rem move /Y serialization.pb.h body.pb.h
rem copy warningguardstart.cc+body.pb.h+warningguardend.cc /a serialization.pb.h /b
rem del body.pb.h