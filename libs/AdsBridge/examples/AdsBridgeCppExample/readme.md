# AdsBridge C++ Example

This example demonstrates how to build the AdsBridge C++ project using CMake.

## Prerequisites
- CMake 3.21 or newer
- C++ compiler supporting C++20

## Building the example

### Windows - one-liner

```pwsh
cd examples/AdsBridgeCppExample
build.bat
```

### Cross-platform - manual commands

```bat
cd examples/AdsBridgeCppExample
cmake -B ./build -DCMAKE_PREFIX_PATH="../../"
cmake --build ./build
```

`build/` is generated during compilation and should be excluded from VCS.

---

## Consuming AdsBridge from CMake project

1. Point `CMAKE_PREFIX_PATH` at the *root* of the SDK (the folder that contains `include/`, `lib/`, and `share/cmake/`):
```bash
cmake -S . -B build -DCMAKE_PREFIX_PATH="/absolute/path/to/dev"
```

2. Link to the exported target:
```cmake
find_package(AdsBridge REQUIRED)

add_executable(my_app src/main.cpp)
target_link_libraries(my_app PRIVATE Radiy::AdsBridge)
```

---

## Known issues

| Symptom | Cause & fix |
|---------|-------------|
| "The C++ compiler is not able to compile a simple test program."| Put the repo closer to the drive root (e.g. `C:\ws\dev`). <br>Alternatively, build folder paths can exceed the legacy 260-character limit. Enable long paths once on the system:<br>- Group Policy: *Computer Configuration -> Administrative Templates -> System -> Filesystem -> Enable Win32 long paths* -> **Enabled**<br>- or for Git repos: `git config --system core.longpaths true` |
