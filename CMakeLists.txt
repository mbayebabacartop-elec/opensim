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

add_library(openlab_electrical_engine STATIC
    engines/electrical/src/rl_engine.cpp
    engines/electrical/src/dc_motor_engine.cpp
    engines/electrical/src/linear_circuit_engine.cpp
)
target_include_directories(openlab_electrical_engine PUBLIC engines/electrical/include)
target_link_libraries(openlab_electrical_engine PUBLIC openlab_core)
target_compile_features(openlab_electrical_engine PUBLIC cxx_std_20)

if(MSVC)
    target_compile_options(openlab_core PRIVATE /W4 /permissive- /EHsc)
    target_compile_options(openlab_electrical_engine PRIVATE /W4 /permissive- /EHsc)
    target_compile_options(openlab_miniz PRIVATE /W3)
else()
    target_compile_options(openlab_core PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(openlab_electrical_engine PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(openlab_miniz PRIVATE -Wall -Wextra -Wpedantic)
endif()

qt_add_executable(openlab_app WIN32
    app/main.cpp
    app/main_window.cpp
    app/main_window.hpp
    app/lab_controller.cpp
    app/lab_controller.hpp
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
    openlab_electrical_engine
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
)
target_compile_features(openlab_app PRIVATE cxx_std_20)

if(MSVC)
    target_compile_options(openlab_app PRIVATE /W4 /permissive- /EHsc /utf-8)
else()
    target_compile_options(openlab_app PRIVATE -Wall -Wextra -Wpedantic)
endif()

if(BUILD_TESTING)
    add_executable(openlab_core_smoke tests/core_smoke.cpp)
    target_link_libraries(openlab_core_smoke PRIVATE openlab_core)
    add_test(NAME openlab.core.smoke COMMAND openlab_core_smoke)
    add_executable(openlab_electrical_smoke tests/electrical_smoke.cpp)
    target_link_libraries(openlab_electrical_smoke PRIVATE openlab_electrical_engine)
    add_test(NAME openlab.electrical.rl COMMAND openlab_electrical_smoke)
    add_executable(openlab_dc_motor_smoke tests/dc_motor_smoke.cpp)
    target_link_libraries(openlab_dc_motor_smoke PRIVATE openlab_electrical_engine)
    add_test(NAME openlab.electrical.dc_motor COMMAND openlab_dc_motor_smoke)
    add_executable(openlab_mna_smoke tests/mna_smoke.cpp)
    target_link_libraries(openlab_mna_smoke PRIVATE openlab_electrical_engine)
    add_test(NAME openlab.electrical.mna COMMAND openlab_mna_smoke)
    add_executable(openlab_app_controller_smoke
        tests/app_controller_smoke.cpp
        app/lab_controller.cpp
        app/lab_controller.hpp
    )
    target_link_libraries(openlab_app_controller_smoke PRIVATE openlab_electrical_engine Qt6::Core)
    target_include_directories(openlab_app_controller_smoke PRIVATE app)
    target_compile_features(openlab_app_controller_smoke PRIVATE cxx_std_20)
    if(MSVC)
        target_compile_options(openlab_app_controller_smoke PRIVATE /W4 /permissive- /EHsc /utf-8)
    endif()
    add_test(NAME openlab.app.controller COMMAND openlab_app_controller_smoke)
    set_tests_properties(openlab.app.controller PROPERTIES
        ENVIRONMENT_MODIFICATION "PATH=path_list_prepend:${CMAKE_PREFIX_PATH}/bin"
    )
endif()
