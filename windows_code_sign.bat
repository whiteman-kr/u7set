@echo off

REM Check if an argument is provided
if "%1"=="" (
    echo Error: No build no argument provided.
    exit /b 1
)

set CI_PIPELINE_ID=%1

REM Save the current directory
set "CURRENT_DIR=%CD%"

REM Sign software
set "SIGNTOOL=C:\Program Files (x86)\Windows Kits\10\bin\10.0.22621.0\x86\signtool.exe"
"%SIGNTOOL%" sign /v /a /t http://time.certum.pl/ /fd SHA512 ./bin/release/AppDataSrv.exe ./bin/release/ArchSrv.exe ./bin/release/CfgSrv.exe ./bin/release/mconf.exe ./bin/release/Metrology.exe ./bin/release/Monitor.exe ./bin/release/scm.exe ./bin/release/TuningClient.exe ./bin/release/TuningSrv.exe ./bin/release/u7.exe ./bin/release/SimulatorTests.exe ./bin/release/SimulatorConsole.exe ./bin/release/BuilderConsole.exe ./bin/release/TestSuite.exe ./bin/release/TestSuiteConsole.exe ./bin/release/Simulator.exe ./bin/release/MetrologyTests.exe ./bin/release/GatewaySrv.exe ./bin/release/u7databasetests.exe ./bin/release/ClientTests.exe


if %ERRORLEVEL% NEQ 0 (
    echo Error: signtool encountered an error with exit code %ERRORLEVEL%.
    exit /b %ERRORLEVEL%
)

REM Create installation
cd u7setinstall
call createinstall.bat
cd ..

REM Sign installation
"%SIGNTOOL%" sign /v /a /t http://time.certum.pl/ /fd SHA512 ./bin/*.exe
rem Check the exit code of signtool
if %ERRORLEVEL% NEQ 0 (
    echo Error: signtool encountered an error with exit code %ERRORLEVEL%.
    exit /b %ERRORLEVEL%
)

REM Restore the original directory
cd /d "%CURRENT_DIR%"

:Done
