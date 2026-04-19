cmake_minimum_required(VERSION 3.21)
project(nexgen_tray VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Network)

add_subdirectory(external/nexgen_qt_sys)
add_subdirectory(external/nexgen_qt_themeset)

# Submodules expected:
# external/nexgen_qt_sys
# external/nexgen_qt_themeset

add_executable(nexgen_tray
  src/main.cpp
)

target_link_libraries(nexgen_tray PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Network
  nexgen::qt_sys
  nexgen::qt_themeset
)
