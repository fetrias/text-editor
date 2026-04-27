cmake_minimum_required(VERSION 3.10)

project(text_editor VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(text_editor)

target_sources(text_editor
    PRIVATE
        src/main.cpp
        src/document.cpp
        src/syntax_highlighter.cpp
        src/editor.cpp
        src/pomodoro_timer.cpp
)

target_include_directories(text_editor
    PRIVATE
        include
)
