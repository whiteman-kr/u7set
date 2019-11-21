set RPCT_VERSION="0.8.1666"

@echo off

echo --------------- Creating installer for RPCT ------------------

del u7setinstall.exe
del config\config.xml

echo Software Version is %RPCT_VERSION%
rem pause 

powershell -Command "(gc config\config.in) -replace '1.0.0', '%RPCT_VERSION%' | Out-File -encoding ASCII config\config.xml"

echo --------------- Creating data folders ------------------

md packages\u7set\data
md packages\u7set.rpct\data
md packages\u7set.srv\data
md packages\u7set.monitor\data
md packages\u7set.tuning\data
md packages\u7set.metrology\data

echo --------------- Copying Source Files ------------------

xcopy ..\bin\release\*.dll packages\u7set\data /sy
copy ..\bin\release\vcredist_x64.exe packages\u7set\data

copy ..\bin\release\u7.exe packages\u7set.rpct\data
copy ..\bin\release\mconf.exe packages\u7set.rpct\data

copy ..\bin\release\CfgSrv.exe packages\u7set.srv\data
copy ..\bin\release\scm.exe packages\u7set.srv\data

copy ..\bin\release\ArchSrv.exe packages\u7set.monitor\data
copy ..\bin\release\AppDataSrv.exe packages\u7set.monitor\data
copy ..\bin\release\Monitor.exe packages\u7set.monitor\data

copy ..\bin\release\TuningClient.exe packages\u7set.tuning\data
copy ..\bin\release\TuningSrv.exe packages\u7set.tuning\data

copy ..\bin\release\Metrology.exe packages\u7set.metrology\data


echo --------------- Building the installer ------------------

binarycreator.exe --offline-only -c config\config.xml -p packages ..\bin\u7setinstall.exe

rem echo Finished creating installer. Data files will be deleted now.
rem pause

echo --------------- Deleting data folders ------------------

rd packages\u7set\data /s /q
rd packages\u7set.rpct\data /s /q
rd packages\u7set.srv\data /s /q
rd packages\u7set.monitor\data /s /q
rd packages\u7set.tuning\data /s /q
rd packages\u7set.metrology\data /s /q

del config\config.xml

echo --------------- Done ------------------
