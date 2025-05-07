# vcpkg triplet to support address sanitizer

set(PORT_DEBUG ON)

set(VCPKG_TARGET_ARCHITECTURE x64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE dynamic)

# Add sanitizer flags
set(VCPKG_CMAKE_C_FLAGS_RELEASE "${VCPKG_CMAKE_C_FLAGS_RELEASE} -fsanitize=address")
set(VCPKG_CMAKE_C_FLAGS_DEBUG   "${VCPKG_CMAKE_C_FLAGS_DEBUG}   -fsanitize=address")

set(VCPKG_CMAKE_CXX_FLAGS_RELEASE "${VCPKG_CMAKE_CXX_FLAGS_RELEASE} -fsanitize=address")
set(VCPKG_CMAKE_CXX_FLAGS_DEBUG   "${VCPKG_CMAKE_CXX_FLAGS_DEBUG}   -fsanitize=address")

set(VCPKG_LINKER_FLAGS_RELEASE "${VCPKG_LINKER_FLAGS_RELEASE} -fsanitize=address")
set(VCPKG_LINKER_FLAGS_DEBUG   "${VCPKG_LINKER_FLAGS_DEBUG}   -fsanitize=address")

