# VtkRenderer

A C++ Qt + VTK application project.

## Prerequisites

- MSYS2 with UCRT64/MINGW64 toolchain
- CMake (3.20 or higher)
- Qt6 (Core, Widgets)
- VTK (with Qt support)

## Setup

1. Install dependencies via MSYS2:
   ```bash
   pacman -S mingw-w64-ucrt-x86_64-qt6-base
   pacman -S mingw-w64-ucrt-x86_64-vtk
   pacman -S mingw-w64-ucrt-x86_64-cmake
   pacman -S mingw-w64-ucrt-x86_64-gdb
   ```

2. Configure the project:
   - Run the "CMake Configure" task in VS Code, or
   - Run manually:
     ```bash
     cmake -B build -S . -G "MinGW Makefiles" -DCMAKE_C_COMPILER=C:/msys64/mingw64/bin/gcc.exe -DCMAKE_CXX_COMPILER=C:/msys64/mingw64/bin/g++.exe
     ```

3. Build the project:
   - Use the "CMake Build" task in VS Code, or
   - Run: `cmake --build build`

4. Run the application:
   - Use the "Debug CMake App" launch configuration in VS Code

## Project Structure

```
VtkRenderer/
├── CMakeLists.txt      # CMake configuration
├── src/
│   └── main.cpp        # Main application source
├── .vscode/
│   ├── launch.json     # Debug configurations
│   └── tasks.json      # Build tasks
└── README.md           # This file
```

## Notes

- The project uses Qt6 and VTK with Qt integration
- Output executable: `build/bin/VtkRenderer.exe`
- Make sure Qt6 and VTK are properly installed and findable by CMake

