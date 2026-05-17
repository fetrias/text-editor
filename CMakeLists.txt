cmake_minimum_required(VERSION 3.16)

project(texteditor LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Core5Compat)

add_executable(texteditor WIN32
    src/main.cpp
    src/mainwindow.cpp
    src/codeeditor.cpp
    src/syntaxhighlighter.cpp
    src/pomodorotimer.cpp
    include/mainwindow.h
    include/codeeditor.h
    include/syntaxhighlighter.h
    include/pomodorotimer.h
)

target_include_directories(texteditor PRIVATE include)

target_link_libraries(texteditor
    PRIVATE
        Qt6::Core
        Qt6::Widgets
        Qt6::Core5Compat
)
