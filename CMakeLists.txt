cmake_minimum_required(VERSION 3.22)

project(SimulIDE
    VERSION 2.0.0
    DESCRIPTION "SimulIDE Electronic Circuit Simulator"
    LANGUAGES CXX ASM
)

# ── C++ standard ─────────────────────────────────────────────────────────────
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ── Qt6 ──────────────────────────────────────────────────────────────────────
find_package(Qt6 6.2 REQUIRED COMPONENTS
    Core
    Gui
    Widgets
    Svg
    Xml
    Concurrent
    SerialPort
    Multimedia
    LinguistTools   # for qt6_add_translations / qt6_add_lrelease
)

qt_standard_project_setup()   # automatic moc / uic / rcc

# ── Version / build-date defines ─────────────────────────────────────────────
set(APP_RELEASE "")

if(CMAKE_HOST_WIN32)
    execute_process(COMMAND powershell -Command "get-date -format yy-MM-dd"
        OUTPUT_VARIABLE REV_NO    OUTPUT_STRIP_TRAILING_WHITESPACE)
    execute_process(COMMAND powershell -Command "get-date -format dd-MM-yy"
        OUTPUT_VARIABLE BUILD_DATE OUTPUT_STRIP_TRAILING_WHITESPACE)
else()
    execute_process(COMMAND date "+%y%m%d"
        OUTPUT_VARIABLE REV_NO    OUTPUT_STRIP_TRAILING_WHITESPACE)
    execute_process(COMMAND date "+%d-%m-%y"
        OUTPUT_VARIABLE BUILD_DATE OUTPUT_STRIP_TRAILING_WHITESPACE)
endif()

# ── Sources ───────────────────────────────────────────────────────────────────
file(GLOB_RECURSE SIMULIDE_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
)
file(GLOB_RECURSE SIMULIDE_HEADERS CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/src/*.h"
)

# arm64 / aarch64 — AngelScript JIT helper
if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm64|aarch64" OR
   CMAKE_C_COMPILER MATCHES "aarch64")
    if(APPLE)
        list(APPEND SIMULIDE_SOURCES
            "${CMAKE_SOURCE_DIR}/src/angel/src/as_callfunc_arm64_xcode.S")
    else()
        list(APPEND SIMULIDE_SOURCES
            "${CMAKE_SOURCE_DIR}/src/angel/src/as_callfunc_arm64_gcc.S")
    endif()
endif()

# ── Target ────────────────────────────────────────────────────────────────────
qt_add_executable(simulide
    ${SIMULIDE_SOURCES}
    ${SIMULIDE_HEADERS}
)

# ── Qt resources (.qrc) ───────────────────────────────────────────────────────
# Invoke rcc explicitly from the .qrc directory so that the relative paths
# inside application.qrc (../resources/..., ../src/icons/...) resolve correctly.
set(QRC_SRC "${CMAKE_BINARY_DIR}/qrc_application.cpp")
add_custom_command(
    OUTPUT  "${QRC_SRC}"
    COMMAND Qt6::rcc
            --name application
            --output "${QRC_SRC}"
            application.qrc
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/src"
    DEPENDS "${CMAKE_SOURCE_DIR}/src/application.qrc" ${SIMULIDE_QM_FILES}
    COMMENT "Compiling Qt resources (application.qrc)"
    VERBATIM
)
target_sources(simulide PRIVATE "${QRC_SRC}")

# ── Translations ──────────────────────────────────────────────────────────────
# qt6_add_translations must be called AFTER qt_add_executable
set(SIMULIDE_TS_FILES
    "${CMAKE_SOURCE_DIR}/resources/translations/simulide.ts"
    "${CMAKE_SOURCE_DIR}/resources/translations/simulide_cz.ts"
    "${CMAKE_SOURCE_DIR}/resources/translations/simulide_de.ts"
    "${CMAKE_SOURCE_DIR}/resources/translations/simulide_en.ts"
    "${CMAKE_SOURCE_DIR}/resources/translations/simulide_es.ts"
    "${CMAKE_SOURCE_DIR}/resources/translations/simulide_pt_BR.ts"
    "${CMAKE_SOURCE_DIR}/resources/translations/simulide_zh_CN.ts"
    "${CMAKE_SOURCE_DIR}/resources/translations/simulide_zh_TW.ts"
    "${CMAKE_SOURCE_DIR}/resources/translations/qt/qt_cz.ts"
    "${CMAKE_SOURCE_DIR}/resources/translations/qt/qt_de.ts"
    "${CMAKE_SOURCE_DIR}/resources/translations/qt/qt_en.ts"
    "${CMAKE_SOURCE_DIR}/resources/translations/qt/qt_es.ts"
    "${CMAKE_SOURCE_DIR}/resources/translations/qt/qt_pt_BR.ts"
    "${CMAKE_SOURCE_DIR}/resources/translations/qt/qt_zh_CN.ts"
    "${CMAKE_SOURCE_DIR}/resources/translations/qt/qt_zh_TW.ts"
)

# Generate .qm files into resources/qm/ (same location the .qrc expects them)
# so that rcc can embed them. lrelease6 must exist on the build host.
find_program(LRELEASE_EXECUTABLE NAMES lrelease6 lrelease-qt6 lrelease
    HINTS "${Qt6_DIR}/../../../bin" "${Qt6_DIR}/../bin" "/usr/bin"
    REQUIRED
)

set(SIMULIDE_QM_FILES "")
foreach(_ts_file ${SIMULIDE_TS_FILES})
    get_filename_component(_ts_name "${_ts_file}" NAME_WE)
    set(_qm_out "${CMAKE_SOURCE_DIR}/resources/qm/${_ts_name}.qm")
    add_custom_command(
        OUTPUT  "${_qm_out}"
        COMMAND "${LRELEASE_EXECUTABLE}" "${_ts_file}" -qm "${_qm_out}"
        DEPENDS "${_ts_file}"
        COMMENT "lrelease ${_ts_name}.ts -> ${_ts_name}.qm"
        VERBATIM
    )
    list(APPEND SIMULIDE_QM_FILES "${_qm_out}")
endforeach()
add_custom_target(SimulIDE_lrelease ALL DEPENDS ${SIMULIDE_QM_FILES})

# ── Include directories ───────────────────────────────────────────────────────
target_include_directories(simulide PRIVATE
    src
    src/components
    src/components/active
    src/components/connectors
    src/components/graphical
    src/components/logic
    src/components/meters
    src/components/micro
    src/components/other
    src/components/other/truthtable
    src/components/outputs
    src/components/outputs/displays
    src/components/outputs/leds
    src/components/outputs/motors
    src/components/passive
    src/components/passive/reactive
    src/components/passive/resistors
    src/components/passive/resist_sensors
    src/components/sources
    src/components/subcircuits
    src/components/switches
    src/gui
    src/gui/appdialogs
    src/gui/circuitwidget
    src/gui/componentlist
    src/gui/dataplotwidget
    src/gui/editorwidget
    src/gui/editorwidget/debuggers
    src/gui/editorwidget/dialogs
    src/gui/filebrowser
    src/gui/memory
    src/gui/properties
    src/gui/serial
    src/gui/testing
    src/simulator
    src/simulator/elements
    src/simulator/elements/active
    src/simulator/elements/outputs
    src/simulator/elements/passive
    src/microsim
    src/microsim/cores
    src/microsim/cores/avr
    src/microsim/cores/i51
    src/microsim/cores/pic
    src/microsim/cores/mcs65
    src/microsim/cores/z80
    src/microsim/cores/scripted
    src/microsim/cores/qemu
    src/microsim/cores/qemu/esp32
    src/microsim/cores/qemu/stm32
    src/microsim/modules
    src/microsim/modules/memory
    src/microsim/modules/usart
    src/microsim/modules/onewire
    src/microsim/modules/twi
    src/microsim/modules/tcp
    src/microsim/modules/spi
    src/microsim/modules/script
    src/angel/include
    src/angel/JIT
    src/angel/src
)

# ── Link libraries ────────────────────────────────────────────────────────────
target_link_libraries(simulide PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Svg
    Qt6::Xml
    Qt6::Concurrent
    Qt6::SerialPort
    Qt6::Multimedia
    z
)

# ── Compiler flags ────────────────────────────────────────────────────────────
target_compile_options(simulide PRIVATE
    -Wno-unused-parameter
    -Wno-implicit-fallthrough
    -fno-strict-aliasing
    -Wno-cast-function-type
    -Wno-deprecated-copy
    -Wno-invalid-offsetof
    $<$<CONFIG:Release>:-Ofast>
    $<$<CONFIG:Debug>:-O0>
)

# ── Compile definitions ───────────────────────────────────────────────────────
target_compile_definitions(simulide PRIVATE
    $<$<CONFIG:Debug>:_GLIBCXX_ASSERTIONS>
    REVNO="${REV_NO}"
    APP_VERSION="${PROJECT_VERSION}${APP_RELEASE}"
    BUILDDATE="${BUILD_DATE}"
)

# ── Platform-specific ─────────────────────────────────────────────────────────
if(WIN32)
    target_link_libraries(simulide PRIVATE wsock32)
    set_target_properties(simulide PROPERTIES WIN32_EXECUTABLE TRUE)
    # Windows icon via .rc — only add if the file exists
    if(EXISTS "${CMAKE_SOURCE_DIR}/resources/icons/simulide.rc")
        target_sources(simulide PRIVATE
            "${CMAKE_SOURCE_DIR}/resources/icons/simulide.rc")
    endif()
endif()

if(APPLE)
    set_target_properties(simulide PROPERTIES
        MACOSX_BUNDLE              TRUE
        MACOSX_BUNDLE_ICON_FILE    simulide.icns
        MACOSX_BUNDLE_GUI_IDENTIFIER "org.simulide.SimulIDE"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
    )
    set(APP_ICON "${CMAKE_SOURCE_DIR}/resources/icons/simulide.icns")
    set_source_files_properties(${APP_ICON}
        PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
    target_sources(simulide PRIVATE ${APP_ICON})
endif()

# ── Install ───────────────────────────────────────────────────────────────────
include(GNUInstallDirs)

install(TARGETS simulide
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    BUNDLE  DESTINATION .
)
install(DIRECTORY "${CMAKE_SOURCE_DIR}/resources/data/"
    DESTINATION "${CMAKE_INSTALL_DATADIR}/simulide/data"
)
install(FILES ${SIMULIDE_QM_FILES}
    DESTINATION "${CMAKE_INSTALL_DATADIR}/simulide/qm"
)
if(EXISTS "${CMAKE_SOURCE_DIR}/resources/simulide.desktop")
    install(FILES "${CMAKE_SOURCE_DIR}/resources/simulide.desktop"
        DESTINATION "${CMAKE_INSTALL_DATADIR}/applications")
endif()
if(EXISTS "${CMAKE_SOURCE_DIR}/resources/simulide-mime.xml")
    install(FILES "${CMAKE_SOURCE_DIR}/resources/simulide-mime.xml"
        DESTINATION "${CMAKE_INSTALL_DATADIR}/mime/packages")
endif()
install(FILES "${CMAKE_SOURCE_DIR}/resources/icons/simulide.png"
    DESTINATION "${CMAKE_INSTALL_DATADIR}/pixmaps"
)

# ── Status messages ───────────────────────────────────────────────────────────
message(STATUS "-----------------------------------")
message(STATUS "  SimulIDE ${PROJECT_VERSION}${APP_RELEASE}")
message(STATUS "  Qt version : ${Qt6_VERSION}")
message(STATUS "  Compiler   : ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "  Build type : ${CMAKE_BUILD_TYPE}")
message(STATUS "  Date       : ${BUILD_DATE}")
message(STATUS "  Install to : ${CMAKE_INSTALL_PREFIX}")
message(STATUS "-----------------------------------")
