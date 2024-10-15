set RPCT_VERSION="%U7SET_FULL_VERSION%"
echo Software Version is %RPCT_VERSION%

powershell -Command "(gc config\config.in) -replace '<Version>1.0.0</Version>', '<Version>%RPCT_VERSION%</Version>' | Out-File -encoding ASCII config\config.xml"

echo --------------- Creating data folders ------------------

md packages\u7set\data
md packages\u7set.develop.rpct\data
md packages\u7set.develop.rpct\data\scripthelp
md packages\u7set.develop.rpct\data\scripting
md packages\u7set.develop.rpct.docs\data\docs
md packages\u7set.develop.rpct.docs\data\docs\Appendixes
md packages\u7set.mats.cfgsrv\data
md packages\u7set.mats.appdatasrv\data
md packages\u7set.mats.gatewaysrv\data
md packages\u7set.mats.archsrv\data
md packages\u7set.mats.tunsrv\data
md packages\u7set.mats.scm\data
md packages\u7set.mats.monitor\data
md packages\u7set.mats.monitor.docs\data\docs
md packages\u7set.mats.tuningclient\data
md packages\u7set.mats.tuningclient.docs\data\docs
md packages\u7set.tools.metrology\data
md packages\u7set.tools.mconf\data
md packages\u7set.tools.testsuite\data

echo --------------- Copying Source Files ------------------

xcopy ..\bin\release\qt*.qm packages\u7set\data /sy
xcopy ..\bin\release\ClientLib*.qm packages\u7set\data /sy
xcopy ..\bin\release\TrendView*.qm packages\u7set\data /sy
xcopy ..\bin\release\UiLib*.qm packages\u7set\data /sy
xcopy ..\bin\release\UtilsLib*.qm packages\u7set\data /sy
xcopy ..\bin\release\SchemaClientLib*.qm packages\u7set\data /sy
xcopy ..\bin\release\AppSignalLists*.qm packages\u7set\data /sy

xcopy ..\bin\release\*.dll packages\u7set\data /sy
copy ..\bin\release\vc_redist.x64.exe packages\u7set\data

copy ..\bin\release\u7.exe packages\u7set.develop.rpct\data

md packages\u7set\data\license
copy ..\bin\release\license\*.* packages\u7set\data\license

xcopy ..\bin\release\scripthelp packages\u7set.develop.rpct\data\scripthelp /sy
xcopy ..\bin\release\scripting packages\u7set.develop.rpct\data\scripting /sy

copy ..\bin\release\docs\D11.5_AFBL_RM.pdf packages\u7set.develop.rpct.docs\data\docs
copy ..\bin\release\docs\D11.6_RPCT-UM.pdf packages\u7set.develop.rpct.docs\data\docs
copy "..\bin\release\docs\Appendixes\D11.6 RPCT User Manual Appendix A Warnings and Errors List.pdf" packages\u7set.develop.rpct.docs\data\docs\Appendixes
copy "..\bin\release\docs\Appendixes\D11.6 RPCT User Manual Appendix B Build Directory and Output Bitstream File Description.pdf" packages\u7set.develop.rpct.docs\data\docs\Appendixes
copy "..\bin\release\docs\Appendixes\D11.6 RPCT User Manual Appendix C JavaScript Manual.pdf" packages\u7set.develop.rpct.docs\data\docs\Appendixes
copy "..\bin\release\docs\Installing and configuring RPCT.pdf" packages\u7set.develop.rpct.docs\data\docs
copy "..\bin\release\docs\RPCT Quick Start Guide.pdf" packages\u7set.develop.rpct.docs\data\docs
copy ..\bin\release\CfgSrv.exe packages\u7set.mats.cfgsrv\data
copy ..\bin\release\AppDataSrv.exe packages\u7set.mats.appdatasrv\data
copy ..\bin\release\ArchSrv.exe packages\u7set.mats.archsrv\data
copy ..\bin\release\TuningSrv.exe packages\u7set.mats.tunsrv\data
copy ..\bin\release\GatewaySrv.exe packages\u7set.mats.gatewaysrv\data
copy ..\bin\release\scm.exe packages\u7set.mats.scm\data
copy ..\bin\release\Monitor.exe packages\u7set.mats.monitor\data
xcopy ..\bin\release\Monitor*.qm packages\u7set.mats.monitor\data /sy
copy ..\bin\release\docs\D11.8_FSC_MATS_User_Manual.pdf packages\u7set.mats.monitor.docs\data\docs
copy ..\bin\release\TuningClient.exe packages\u7set.mats.tuningclient\data
xcopy ..\bin\release\TuningClient*.qm packages\u7set.mats.tuningclient\data /sy
copy ..\bin\release\docs\D11.9_FSC_Tuning_User_Manual.pdf packages\u7set.mats.tuningclient.docs\data\docs

copy ..\bin\release\Metrology.exe packages\u7set.tools.metrology\data
copy ..\bin\release\mconf.exe packages\u7set.tools.mconf\data
copy ..\bin\release\TestSuite.exe packages\u7set.tools.testsuite\data
copy ..\bin\release\TestSuiteConsole.exe packages\u7set.tools.testsuite\data
xcopy ..\bin\release\TestSuite*.qm packages\u7set.tools.testsuite\data /sy

echo --------------- Building the installer ------------------

binarycreator.exe --offline-only -c config\config.xml -p packages ..\bin\u7setinstall-%RPCT_VERSION%-PPID_%CI_PIPELINE_ID%-%CI_RELEASE_TYPE%-%CI_COMMIT_REF_SLUG%_%CI_COMMIT_SHA%.exe

echo --------------- Done ------------------
