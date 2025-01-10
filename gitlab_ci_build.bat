call "C:/Program Files (x86)/Microsoft Visual Studio/2022/BuildTools/VC/Auxiliary/Build/vcvarsall.bat" amd64 &&^
cl.exe &&^
%QT_BIN_DIR%/bin/qmake --version &&^
cmake --version &&^
cmake -S . -B ./build -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=%QT_BIN_DIR% -DBIN_OUT_DIR=%BIN_OUT_DIR% &&^
cmake --build ./build --verbose
