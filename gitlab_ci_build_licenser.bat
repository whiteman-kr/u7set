@echo on

REM Build LicenserBlacklistGenerator, generate EmbeddedBlacklist by it, delete LicenserBlacklistGenerator.exe

git -c credential.helper= clone --depth 1 "http://gitlab-ci-token:%CI_JOB_TOKEN%@192.168.75.11/radiy/licenses.git" licenses

call "C:/Program Files (x86)/Microsoft Visual Studio/2022/BuildTools/VC/Auxiliary/Build/vcvarsall.bat" amd64 &&^
cl.exe &&^
%QT_BIN_DIR%/bin/qmake --version &&^
cmake --version &&^
cmake -S . -B ./build -G Ninja -DCMAKE_C_COMPILER="cl" -DCMAKE_CXX_COMPILER="cl" -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=%QT_BIN_DIR% -DBIN_OUT_DIR=%BIN_OUT_DIR% -DBUILD_LICENSER_BLACKLIST_GENERATOR=ON &&^
cmake --build ./build --target LicenserBlacklistGenerator

pushd %BIN_OUT_DIR%
LicenserBlacklistGenerator.exe --generate-blacklist %CI_PROJECT_DIR%\licenses %CI_PROJECT_DIR%\build\EmbeddedBlacklist.h
popd

type %CI_PROJECT_DIR%\build\EmbeddedBlacklist.h

del /F %BIN_OUT_DIR%\LicenserBlacklistGenerator.exe