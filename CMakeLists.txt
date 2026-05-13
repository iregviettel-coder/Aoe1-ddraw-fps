cmake_minimum_required(VERSION 3.20)
project(ddraw_fps_overlay CXX)

# ── Must be 32-bit  (AoE1 is a Win32 process) ────────────────
# MSVC:   cmake -B build -A Win32
# MinGW:  cmake -B build -DCMAKE_TOOLCHAIN_FILE=cmake/mingw32.cmake

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── Static runtime: no VCRUNTIME dependency on target machine ─
if(MSVC)
    cmake_policy(SET CMP0091 NEW)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

# ── Source files ──────────────────────────────────────────────
add_library(ddraw SHARED
    dllmain.cpp
    ddraw7_wrapper.cpp
    surface_wrapper.cpp
)

# ── Windows SDK paths (ddraw.h / ddraw.lib) ───────────────────
# Modern Windows SDK (8.1+) ships ddraw.h under:
#   $ENV{WindowsSdkDir}\Include\<ver>\um\
#   $ENV{WindowsSdkDir}\Lib\<ver>\um\x86\
# ilammy/msvc-dev-cmd and standard VS installs set these env vars.
if(DEFINED ENV{WindowsSdkDir} AND DEFINED ENV{WindowsSDKVersion})
    set(WINSDK_INC "$ENV{WindowsSdkDir}Include\\$ENV{WindowsSDKVersion}um")
    set(WINSDK_LIB "$ENV{WindowsSdkDir}Lib\\$ENV{WindowsSDKVersion}um\\x86")
    target_include_directories(ddraw PRIVATE "${WINSDK_INC}")
    target_link_directories(ddraw PRIVATE "${WINSDK_LIB}")
    message(STATUS "Windows SDK um include: ${WINSDK_INC}")
    message(STATUS "Windows SDK um lib x86: ${WINSDK_LIB}")
else()
    # Fallback: legacy DirectX SDK June 2010
    set(DXSDK_FALLBACK "C:/Program Files (x86)/Microsoft DirectX SDK (June 2010)")
    if(EXISTS "${DXSDK_FALLBACK}")
        target_include_directories(ddraw PRIVATE "${DXSDK_FALLBACK}/Include")
        target_link_directories(ddraw PRIVATE "${DXSDK_FALLBACK}/Lib/x86")
        message(STATUS "Using legacy DXSDK: ${DXSDK_FALLBACK}")
    endif()
endif()

# ── Link ──────────────────────────────────────────────────────
target_link_libraries(ddraw PRIVATE
    ddraw
    dxguid
    gdi32
    user32
    kernel32
)

# Output name must be exactly "ddraw" so the game loads it
set_target_properties(ddraw PROPERTIES OUTPUT_NAME "ddraw")

# ── Install ───────────────────────────────────────────────────
install(TARGETS ddraw DESTINATION .)
