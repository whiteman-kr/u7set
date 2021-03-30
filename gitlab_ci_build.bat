call "C:/Program Files (x86)/Microsoft Visual Studio/2019/BuildTools/VC/Auxiliary/Build/vcvarsall.bat" x86_amd64 &&^
cl.exe && %QT_BIN_DIR%/bin/qmake --version &&^
%QT_BIN_DIR%/bin/qmake.exe %CI_PROJECT_DIR%/u7set.pro -spec win32-msvc &&^
C:/Qt/Tools/QtCreator/bin/jom.exe qmake_all &&^
C:/Qt/Tools/QtCreator/bin/jom.exe
