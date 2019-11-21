set RPCT_VERSION="0.8.%CI_PIPELINE_ID%"
echo Software Version is %RPCT_VERSION%

powershell -Command "(gc u7setinstall\config\config.in) -replace '<Version>1.0.0</Version>', '<Version>%RPCT_VERSION%</Version>' | Out-File -encoding ASCII u7setinstall\config\config.xml"

echo --------------- Creating data folders ------------------

md u7setinstall\packages\u7set\data
md u7setinstall\packages\u7set.rpct\data
md u7setinstall\packages\u7set.srv\data
md u7setinstall\packages\u7set.monitor\data
md u7setinstall\packages\u7set.tuning\data
md u7setinstall\packages\u7set.metrology\data

echo --------------- Copying Source Files ------------------

xcopy bin\release\*.dll u7setinstall\packages\u7set\data /sy
copy bin\release\vcredist_x64.exe u7setinstall\packages\u7set\data

copy bin\release\u7.exe u7setinstall\packages\u7set.rpct\data
copy bin\release\mconf.exe u7setinstall\packages\u7set.rpct\data

copy bin\release\CfgSrv.exe u7setinstall\packages\u7set.srv\data
copy bin\release\scm.exe u7setinstall\packages\u7set.srv\data

copy bin\release\ArchSrv.exe u7setinstall\packages\u7set.monitor\data
copy bin\release\AppDataSrv.exe u7setinstall\packages\u7set.monitor\data
copy bin\release\Monitor.exe u7setinstall\packages\u7set.monitor\data

copy bin\release\TuningClient.exe u7setinstall\packages\u7set.tuning\data
copy bin\release\TuningSrv.exe u7setinstall\packages\u7set.tuning\data

copy bin\release\Metrology.exe u7setinstall\packages\u7set.metrology\data

echo --------------- Building the installer ------------------

binarycreator.exe --offline-only -c u7setinstall\config\config.xml -p u7setinstall\packages bin\u7setinstall_%RPCT_VERSION%_%CI_BUILD_REF_SLUG%_%CI_COMMIT_SHA%.exe

echo --------------- Done ------------------
