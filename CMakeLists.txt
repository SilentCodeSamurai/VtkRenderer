cmake_minimum_required(VERSION 3.20)
project(VtkRenderer VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXECUTABLE_ENABLE_EXPORTS OFF)

# Generate compile_commands.json for clangd/IntelliSense
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_AUTOMOC_COMPILER_PREDEFINES OFF)

# Find Qt6
find_package(Qt6 REQUIRED COMPONENTS Core Widgets)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find VTK
find_package(VTK REQUIRED COMPONENTS
    CommonCore
    CommonDataModel
    FiltersSources
    InteractionStyle
    IOXML
    RenderingAnnotation
    RenderingCore
    RenderingOpenGL2
    GUISupportQt
)

# Source files
set(SOURCES
    src/main.cpp
    src/App.cpp
    src/VtuModelLoader.cpp
    src/ColoringController.cpp
)

# Headers
set(HEADERS
    src/App.h
    src/VtuModelLoader.h
    src/ColoringController.h
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Treat MSYS2 global headers as implicit so CMake doesn't inject them ahead of
# libstdc++ headers (which breaks <cmath> -> #include_next <math.h>).
if(CMAKE_CXX_COMPILER MATCHES "/ucrt64/")
    list(APPEND CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES "C:/msys64/ucrt64/include")
elseif(CMAKE_CXX_COMPILER MATCHES "/mingw64/")
    list(APPEND CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES "C:/msys64/mingw64/include")
endif()

# Link libraries
target_link_libraries(${PROJECT_NAME}
    Qt6::Core
    Qt6::Widgets
    VTK::CommonCore
    VTK::CommonDataModel
    VTK::FiltersSources
    VTK::InteractionStyle
    VTK::IOXML
    VTK::RenderingAnnotation
    VTK::RenderingCore
    VTK::RenderingOpenGL2
    VTK::GUISupportQt
)

# Set output directory
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
