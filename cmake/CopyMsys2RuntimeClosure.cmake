if(NOT DEFINED APP_EXE OR APP_EXE STREQUAL "")
    message(FATAL_ERROR "APP_EXE is required")
endif()
if(NOT DEFINED APP_BIN_DIR OR APP_BIN_DIR STREQUAL "")
    message(FATAL_ERROR "APP_BIN_DIR is required")
endif()
if(NOT DEFINED MSYS2_BIN_DIR OR MSYS2_BIN_DIR STREQUAL "")
    message(FATAL_ERROR "MSYS2_BIN_DIR is required")
endif()
if(NOT DEFINED OBJDUMP_COMMAND OR OBJDUMP_COMMAND STREQUAL "")
    set(OBJDUMP_COMMAND objdump)
endif()

# Be robust to accidentally quoted -DVAR="value" arguments.
foreach(_path_var IN ITEMS APP_EXE APP_BIN_DIR MSYS2_BIN_DIR OBJDUMP_COMMAND)
    string(REGEX REPLACE "^\"(.*)\"$" "\\1" ${_path_var} "${${_path_var}}")
endforeach()

set(_queue "${APP_EXE}")

# Scan all deployed root DLLs too. They may have their own transitive runtime
# dependencies that are not visible directly from APP_EXE imports.
file(GLOB _bin_root_dlls "${APP_BIN_DIR}/*.dll")
foreach(_bin_root_dll IN LISTS _bin_root_dlls)
    list(APPEND _queue "${_bin_root_dll}")
endforeach()

# Also scan deployed plugin DLLs because they are loaded dynamically by Qt.
file(GLOB_RECURSE _plugin_dlls "${APP_BIN_DIR}/*/*.dll")
foreach(_plugin_dll IN LISTS _plugin_dlls)
    list(APPEND _queue "${_plugin_dll}")
endforeach()

set(_scanned "")
set(_copied "")

while(_queue)
    list(GET _queue 0 _current)
    list(REMOVE_AT _queue 0)

    list(FIND _scanned "${_current}" _already_scanned)
    if(NOT _already_scanned EQUAL -1)
        continue()
    endif()
    list(APPEND _scanned "${_current}")

    if(NOT EXISTS "${_current}")
        continue()
    endif()

    execute_process(
        COMMAND "${OBJDUMP_COMMAND}" -p "${_current}"
        OUTPUT_VARIABLE _objdump_out
        RESULT_VARIABLE _objdump_rc
    )
    if(NOT _objdump_rc EQUAL 0)
        continue()
    endif()

    string(REGEX MATCHALL "DLL Name: [^\r\n]+" _dll_lines "${_objdump_out}")
    foreach(_line IN LISTS _dll_lines)
        string(REPLACE "DLL Name: " "" _dll_name "${_line}")
        string(STRIP "${_dll_name}" _dll_name)
        string(TOLOWER "${_dll_name}" _dll_name_lc)

        # Skip Windows API set pseudo-DLL entries.
        if(_dll_name_lc MATCHES "^api-ms-win-" OR _dll_name_lc MATCHES "^ext-ms-win-")
            continue()
        endif()

        set(_dst "${APP_BIN_DIR}/${_dll_name}")
        if(EXISTS "${_dst}")
            list(FIND _scanned "${_dst}" _dst_scanned)
            if(_dst_scanned EQUAL -1)
                list(APPEND _queue "${_dst}")
            endif()
            continue()
        endif()

        set(_src "${MSYS2_BIN_DIR}/${_dll_name}")
        if(EXISTS "${_src}")
            execute_process(
                COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${_src}" "${_dst}"
            )
            list(APPEND _copied "${_dll_name}")
            if(EXISTS "${_dst}")
                list(APPEND _queue "${_dst}")
            endif()
        endif()
    endforeach()
endwhile()

if(_copied)
    list(REMOVE_DUPLICATES _copied)
    foreach(_dll_name IN LISTS _copied)
        message(STATUS "Copied transitive runtime dependency ${_dll_name}")
    endforeach()
endif()

