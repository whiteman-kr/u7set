pushd .
cd %CI_PROJECT_DIR%\bin\release
dir
for /r %%i in (*.exe) do windeployqt %%i
popd



