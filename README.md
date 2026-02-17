# VtkRenderer

VtkRenderer is a Windows desktop application built with Qt6 and VTK for loading and visualizing VTK/VTU data.  
The project includes:

- a GUI executable (`VtkRenderer.exe`) with embedded icon resources,
- automatic runtime deployment for Qt/VTK/MinGW DLLs into `build/bin`,
- MSI packaging via CPack + WiX with shortcut options.

## Build Environment (Windows)

This project is configured as **Windows-only** in CMake and uses **MSYS2 UCRT64**.

### 1) Install MSYS2

Install MSYS2 from the official site:

- https://www.msys2.org/

After installation, open the **MSYS2 UCRT64** shell and update packages:

```bash
pacman -Syu
```

If prompted, close and reopen the shell, then run again:

```bash
pacman -Su
```

### 2) Install build/runtime dependencies (UCRT64)

In the MSYS2 UCRT64 shell:

```bash
pacman -S --needed \
  mingw-w64-ucrt-x86_64-toolchain \
  mingw-w64-ucrt-x86_64-cmake \
  mingw-w64-ucrt-x86_64-ninja \
  mingw-w64-ucrt-x86_64-qt6-base \
  mingw-w64-ucrt-x86_64-vtk \
  mingw-w64-ucrt-x86_64-gdb
```

Notes:

- The VS Code tasks in this repo use `C:/msys64/ucrt64/bin/...` paths.
- `windeployqt` is expected at `C:/msys64/ucrt64/bin/windeployqt.exe`.

### 3) Install WiX toolset binaries for CPack

CPack is configured for WiX v3 layout. Put WiX binaries in:

- `wix/bin/wix311`

At minimum, this folder must contain:

- `candle.exe`
- `light.exe`

If missing, CMake warns and MSI packaging will fail.

## Build and Run

### Option A: VS Code tasks (recommended)

Use tasks from `.vscode/tasks.json` in this order:

1. `CMake Configure`
2. `CMake Build`
3. `CPack` (generates MSI installer)

### Option B: Manual commands (PowerShell)

From the repository root:

```powershell
C:/msys64/ucrt64/bin/cmake.exe -B build -S . -G "MinGW Makefiles" `
  -DCMAKE_C_COMPILER=C:/msys64/ucrt64/bin/gcc.exe `
  -DCMAKE_CXX_COMPILER=C:/msys64/ucrt64/bin/g++.exe `
  -DCMAKE_PREFIX_PATH=C:/msys64/ucrt64 `
  -DCMAKE_MAKE_PROGRAM=C:/msys64/ucrt64/bin/mingw32-make.exe

C:/msys64/ucrt64/bin/cmake.exe --build build --config Release

cd build
C:/msys64/ucrt64/bin/cpack.exe -G WIX -C Release
```

## Outputs

- App executable: `build/bin/VtkRenderer.exe`
- Deployed runtime payload: `build/bin/*` (Qt plugins + required DLLs)
- MSI installer: `build/VtkRenderer-<version>-win64-installer.msi`

## Project Layout (key files)

```text
VtkRenderer/
├── CMakeLists.txt
├── src/
│   ├── Main.cpp
│   └── AppIcon.rc
├── assets/
│   ├── icon.ico
│   └── resources.qrc
├── wix/
│   ├── WixShortcutsPatch.xml
│   └── bin/wix311/   # local WiX binaries
└── .vscode/tasks.json
```

