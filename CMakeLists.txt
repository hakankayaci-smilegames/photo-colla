cmake_minimum_required(VERSION 3.20)

project(PhotoColla
    VERSION 1.0.0
    DESCRIPTION "Professional Hardware-Accelerated Photo Collage Desktop Suite"
    LANGUAGES CXX
)

# ---------------------------------------------------------
# C++ Standard & Compilation Policies
# ---------------------------------------------------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ---------------------------------------------------------
# Qt6 Automation
# ---------------------------------------------------------
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# ---------------------------------------------------------
# CMake Helpers
# ---------------------------------------------------------
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(CompilerWarnings)
include(Sanitizers)

# ---------------------------------------------------------
# Dependencies
# ---------------------------------------------------------
find_package(Qt6 6.4 REQUIRED COMPONENTS
    Core
    Gui
    Widgets
    OpenGLWidgets
    Svg
    Concurrent
)

# ---------------------------------------------------------
# Source Files Definition
# Modules will be expanded incrementally as steps proceed
# ---------------------------------------------------------
set(CORE_SOURCES
    src/core/models/slot.h
    src/core/models/slot.cpp
    src/core/models/auto_layout_engine.h
    src/core/models/auto_layout_engine.cpp
    src/core/models/collage_document.h
    src/core/models/collage_document.cpp
    src/core/commands/collage_commands.h
    src/core/commands/collage_commands.cpp
    src/core/history/command_history.h
    src/core/history/command_history.cpp
)

set(UI_SOURCES
    src/ui/main_window.h
    src/ui/main_window.cpp
    src/ui/splash/splash_screen.h
    src/ui/splash/splash_screen.cpp
    src/ui/canvas/collage_canvas.h
    src/ui/canvas/collage_canvas.cpp
    src/ui/canvas/slot_renderer.h
    src/ui/canvas/slot_renderer.cpp
    src/ui/toolbox/toolbox_panel.h
    src/ui/toolbox/toolbox_panel.cpp
    src/ui/properties/properties_panel.h
    src/ui/properties/properties_panel.cpp
    src/ui/dialogs/export_dialog.h
    src/ui/dialogs/export_dialog.cpp
    src/ui/dialogs/welcome_dialog.h
    src/ui/dialogs/welcome_dialog.cpp
)

set(EXPORT_SOURCES
    src/export/exporter.h
    src/export/exporter.cpp
)

set(RESOURCES
    resources.qrc
)

# ---------------------------------------------------------
# Executable Target
# ---------------------------------------------------------
add_executable(PhotoColla
    src/main.cpp
    ${CORE_SOURCES}
    ${UI_SOURCES}
    ${EXPORT_SOURCES}
    ${RESOURCES}
)

target_include_directories(PhotoColla PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(PhotoColla PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::OpenGLWidgets
    Qt6::Svg
    Qt6::Concurrent
)

# Apply warnings & sanitizers
enable_strict_warnings(PhotoColla)
enable_sanitizers(PhotoColla)

# Interprocedural Optimization (LTO) in Release
include(CheckIPOSupported)
check_ipo_supported(RESULT ipo_supported OUTPUT ipo_error)
if(ipo_supported AND CMAKE_BUILD_TYPE STREQUAL "Release")
    set_property(TARGET PhotoColla PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

# Installation target
install(TARGETS PhotoColla
    RUNTIME DESTINATION bin
)
