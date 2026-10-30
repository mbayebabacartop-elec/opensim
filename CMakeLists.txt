cmake_minimum_required(VERSION 3.25)
project(OpenLabSimulator VERSION 0.1.0 LANGUAGES CXX)

include(CTest)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 6.10 REQUIRED COMPONENTS Widgets)
qt_standard_project_setup()

add_library(openlab_core STATIC
    core/src/version.cpp
)
target_include_directories(openlab_core
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/core/include>
)
target_compile_features(openlab_core PUBLIC cxx_std_20)

if(MSVC)
    target_compile_options(openlab_core PRIVATE /W4 /permissive- /EHsc)
else()
    target_compile_options(openlab_core PRIVATE -Wall -Wextra -Wpedantic)
endif()

qt_add_executable(openlab_app WIN32
    app/main.cpp
    app/main_window.cpp
    app/main_window.hpp
    app/logging.cpp
    app/logging.hpp
)
target_link_libraries(openlab_app PRIVATE openlab_core Qt6::Widgets)
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
