set RPCT_VERSION="0.8.%CI_PIPELINE_ID%"
echo Software Version is %RPCT_VERSION%

powershell -Command "(gc config\config.in) -replace '<Version>1.0.0</Version>', '<Version>%RPCT_VERSION%</Version>' | Out-File -encoding ASCII config\config.xml"

echo --------------- Creating data folders ------------------

md packages\u7set\data
md packages\u7set.rpct\data
md packages\u7set.rpct\data\scripthelp
md packages\u7set.srv\data
md packages\u7set.monitor\data
md packages\u7set.tuning\data
md packages\u7set.metrology\data

echo --------------- Copying Source Files ------------------

xcopy bin\release\*.dll packages\u7set\data /sy
copy bin\release\vcredist_x64.exe packages\u7set\data

xcopy bin\release\scripthelp packages\u7set.rpct\data\scripthelp /sy
copy bin\release\u7.exe packages\u7set.rpct\data
copy bin\release\mconf.exe packages\u7set.rpct\data

copy bin\release\CfgSrv.exe packages\u7set.srv\data
copy bin\release\scm.exe packages\u7set.srv\data

copy bin\release\ArchSrv.exe packages\u7set.monitor\data
copy bin\release\AppDataSrv.exe packages\u7set.monitor\data
copy bin\release\Monitor.exe packages\u7set.monitor\data

copy bin\release\TuningClient.exe packages\u7set.tuning\data
copy bin\release\TuningSrv.exe packages\u7set.tuning\data

copy bin\release\Metrology.exe packages\u7set.metrology\data

echo --------------- Building the installer ------------------

binarycreator.exe --offline-only -c config\config.xml -p packages bin\u7setinstall_%RPCT_VERSION%_%CI_BUILD_REF_SLUG%_%CI_COMMIT_SHA%.exe

echo --------------- Done ------------------
