@echo on

call "C:/Program Files (x86)/Microsoft Visual Studio/2022/BuildTools/VC/Auxiliary/Build/vcvarsall.bat" amd64 &&^
cl.exe &&^
%QT_BIN_DIR%/bin/qmake --version &&^
cmake --version &&^
cmake -S . -B ./build -G Ninja -DCMAKE_C_COMPILER="cl" -DCMAKE_CXX_COMPILER="cl" -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=%QT_BIN_DIR% -DBIN_OUT_DIR=%BIN_OUT_DIR% -DBUILD_LICENSER_BLACKLIST_GENERATOR=OFF &&^
cmake --build ./build --clean-first &&^
cmake --install ./build --prefix %INSTALL_OUT_DIR%  &&^
cmake -S %INSTALL_OUT_DIR%/examples/AdsBridgeExample -B ./TestAdsBridgeBuild -DCMAKE_PREFIX_PATH="%INSTALL_OUT_DIR%" &&^
cmake --build ./TestAdsBridgeBuild