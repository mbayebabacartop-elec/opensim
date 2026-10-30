cmake_minimum_required(VERSION 3.25)
project(OpenLabSimulator VERSION 0.1.0 LANGUAGES C CXX)

include(CTest)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 6.10 REQUIRED COMPONENTS Core Gui Qml Quick QuickControls2)
qt_standard_project_setup(REQUIRES 6.10)

include(GenerateExportHeader)
add_library(openlab_miniz STATIC
    third_party/miniz/miniz.c
    third_party/miniz/miniz_zip.c
    third_party/miniz/miniz_tinfl.c
    third_party/miniz/miniz_tdef.c
)
generate_export_header(openlab_miniz
    EXPORT_FILE_NAME "${CMAKE_CURRENT_BINARY_DIR}/generated/miniz_export.h"
    EXPORT_MACRO_NAME MINIZ_EXPORT
    STATIC_DEFINE MINIZ_STATIC_DEFINE
)
target_compile_definitions(openlab_miniz PUBLIC MINIZ_STATIC_DEFINE PRIVATE MINIZ_NO_STDIO)
target_include_directories(openlab_miniz PUBLIC third_party/miniz "${CMAKE_CURRENT_BINARY_DIR}/generated")

add_library(nlohmann_json INTERFACE)
target_include_directories(nlohmann_json INTERFACE third_party/nlohmann_json)

add_library(openlab_core STATIC
    core/src/version.cpp
    core/src/units.cpp
    core/src/components.cpp
    core/src/project_model.cpp
    core/src/simulation_time.cpp
    core/src/project_store.cpp
)
target_include_directories(openlab_core
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/core/include>
)
target_compile_features(openlab_core PUBLIC cxx_std_20)
target_link_libraries(openlab_core PRIVATE nlohmann_json openlab_miniz)

if(MSVC)
    target_compile_options(openlab_core PRIVATE /W4 /permissive- /EHsc)
    target_compile_options(openlab_miniz PRIVATE /W3)
else()
    target_compile_options(openlab_core PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(openlab_miniz PRIVATE -Wall -Wextra -Wpedantic)
endif()

qt_add_executable(openlab_app WIN32
    app/main.cpp
    app/main_window.cpp
    app/main_window.hpp
    app/logging.cpp
    app/logging.hpp
)
set_source_files_properties(interface/Main.qml PROPERTIES QT_RESOURCE_ALIAS Main.qml)
qt_add_qml_module(openlab_app
    URI OpenLab
    VERSION 1.0
    QML_FILES interface/Main.qml
)
target_link_libraries(openlab_app PRIVATE
    openlab_core
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
)
target_compile_features(openlab_app PRIVATE cxx_std_20)

if(MSVC)
    target_compile_options(openlab_app PRIVATE /W4 /permissive- /EHsc)
else()
    target_compile_options(openlab_app PRIVATE -Wall -Wextra -Wpedantic)
endif()

if(BUILD_TESTING)
    add_executable(openlab_core_smoke tests/core_smoke.cpp)
    target_link_libraries(openlab_core_smoke PRIVATE openlab_core)
    add_test(NAME openlab.core.smoke COMMAND openlab_core_smoke)
endif()
