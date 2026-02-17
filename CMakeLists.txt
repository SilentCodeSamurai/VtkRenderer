cmake_minimum_required(VERSION 3.21)
project(VtkRenderer VERSION 1.0.0 LANGUAGES CXX)

# This project is intentionally Windows-only (Qt/VTK deployment and WiX MSI).
if(NOT WIN32)
    message(FATAL_ERROR "VtkRenderer currently supports only Windows builds.")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXECUTABLE_ENABLE_EXPORTS OFF)

# Generate compile_commands.json for clangd/IntelliSense.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_AUTOMOC_COMPILER_PREDEFINES OFF)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Dependencies.
find_package(Qt6 REQUIRED COMPONENTS Core Widgets)
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

# Keep linked VTK targets centralized and reused.
set(VTK_COMPONENT_TARGETS
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

# Sources.
set(SOURCES
    src/Main.cpp
    src/App.cpp
    src/VtuModelLoader.cpp
    src/ColoringController.cpp
    src/AppIcon.rc
    assets/resources.qrc
)

set(HEADERS
    src/App.h
    src/VtuModelLoader.h
    src/ColoringController.h
)

# Build a GUI subsystem executable (no console window).
add_executable(${PROJECT_NAME} WIN32 ${SOURCES} ${HEADERS})

target_link_libraries(${PROJECT_NAME}
    Qt6::Core
    Qt6::Widgets
    ${VTK_COMPONENT_TARGETS}
)

# Treat MSYS2 global headers as implicit so CMake doesn't inject them ahead of
# libstdc++ headers (which breaks <cmath> -> #include_next <math.h>).
if(CMAKE_CXX_COMPILER MATCHES "/ucrt64/")
    list(APPEND CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES "C:/msys64/ucrt64/include")
elseif(CMAKE_CXX_COMPILER MATCHES "/mingw64/")
    list(APPEND CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES "C:/msys64/mingw64/include")
endif()

# Deployment output root.
set(BIN_OUTPUT "${CMAKE_BINARY_DIR}/bin")
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${BIN_OUTPUT}"
)

# --- Runtime deployment (build tree) ---
set(MSYS2_BIN_PATH "C:/msys64/ucrt64/bin")
set(MSYS2_RUNTIME_SCAN_SCRIPT "${CMAKE_SOURCE_DIR}/cmake/CopyMsys2RuntimeClosure.cmake")

# Deploy Qt runtime/plugins using Qt's official tool.
find_program(WINDEPLOYQT_EXECUTABLE windeployqt PATHS "${MSYS2_BIN_PATH}")
if(WINDEPLOYQT_EXECUTABLE)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND "${WINDEPLOYQT_EXECUTABLE}"
            "$<TARGET_FILE:${PROJECT_NAME}>"
            --dir "${BIN_OUTPUT}"
            --no-compiler-runtime
        COMMENT "Deploying Qt dependencies with windeployqt..."
        VERBATIM
    )
else()
    message(WARNING "windeployqt not found, falling back to manual Qt DLL copy.")
    file(GLOB QT_DLLS "${MSYS2_BIN_PATH}/Qt6*.dll")
    foreach(DLL ${QT_DLLS})
        get_filename_component(DLL_NAME "${DLL}" NAME)
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different "${DLL}" "${BIN_OUTPUT}/"
            COMMENT "Copying ${DLL_NAME}..."
            VERBATIM
        )
    endforeach()
endif()

# Copy direct runtime DLLs known to CMake (requires CMake >= 3.21).
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_RUNTIME_DLLS:${PROJECT_NAME}>
        "${BIN_OUTPUT}/"
    COMMAND_EXPAND_LISTS
    COMMENT "Copying runtime dependencies resolved by CMake..."
    VERBATIM
)

# Resolve remaining transitive MinGW runtime closure from MSYS2 bin.
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND}
        -DAPP_EXE="$<TARGET_FILE:${PROJECT_NAME}>"
        -DAPP_BIN_DIR="${BIN_OUTPUT}"
        -DMSYS2_BIN_DIR="${MSYS2_BIN_PATH}"
        -DOBJDUMP_COMMAND="${CMAKE_OBJDUMP}"
        -P "${MSYS2_RUNTIME_SCAN_SCRIPT}"
    COMMENT "Resolving transitive MinGW runtime dependencies..."
    VERBATIM
)

# GLib can load some modules lazily; keep these explicit safety-net DLLs.
foreach(_extra_glib_dll IN ITEMS "libffi-8.dll" "libgmodule-2.0-0.dll")
    if(EXISTS "${MSYS2_BIN_PATH}/${_extra_glib_dll}")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${MSYS2_BIN_PATH}/${_extra_glib_dll}"
                "${BIN_OUTPUT}/"
            COMMENT "Copying ${_extra_glib_dll}..."
            VERBATIM
        )
    endif()
endforeach()

# --- Install + MSI packaging (WiX) ---
# Package the already deployed runtime payload from build/bin.
install(DIRECTORY "${BIN_OUTPUT}/" DESTINATION "bin" COMPONENT Runtime)

set(CPACK_PACKAGE_NAME "${PROJECT_NAME}")
set(CPACK_PACKAGE_VENDOR "VtkRenderer")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "${PROJECT_NAME}")
set(CPACK_MONOLITHIC_INSTALL ON)

set(CPACK_GENERATOR "WIX")
set(CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME}-${PROJECT_VERSION}-win64-installer")
set(CPACK_INSTALL_CMAKE_PROJECTS "${CMAKE_BINARY_DIR};${PROJECT_NAME};ALL;/")
set(CPACK_WIX_ROOT "${CMAKE_SOURCE_DIR}/wix/bin/wix311")
set(CPACK_WIX_UI_REF "WixUI_InstallDir")
set(CPACK_WIX_PRODUCT_ICON "${CMAKE_SOURCE_DIR}/assets/icon.ico")
set(CPACK_WIX_PROGRAM_MENU_FOLDER "${PROJECT_NAME}")
set(CPACK_WIX_PATCH_FILE "${CMAKE_SOURCE_DIR}/wix/WixShortcutsPatch.xml")

if(NOT EXISTS "${CPACK_WIX_ROOT}/candle.exe")
    message(WARNING "WiX tools not found at ${CPACK_WIX_ROOT}. Install deps to build MSI.")
endif()

include(CPack)
