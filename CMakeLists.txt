cmake_minimum_required(VERSION 3.16)

project(texteditor LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

if (WIN32 AND NOT DEFINED CMAKE_PREFIX_PATH)
    if (EXISTS "C:/Qt/6.11.1/mingw_64/lib/cmake/Qt6")
        list(PREPEND CMAKE_PREFIX_PATH "C:/Qt/6.11.1/mingw_64")
    endif()
endif()

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Core5Compat Test)

set(TEXTEDITOR_SOURCES
    src/mainwindow.cpp
    src/codeeditor.cpp
    src/syntaxhighlighter.cpp
    src/pomodorotimer.cpp
    include/mainwindow.h
    include/codeeditor.h
    include/syntaxhighlighter.h
    include/pomodorotimer.h
)

add_library(texteditor_lib ${TEXTEDITOR_SOURCES})
target_include_directories(texteditor_lib PUBLIC include)
target_link_libraries(texteditor_lib
    PUBLIC
        Qt6::Core
        Qt6::Widgets
        Qt6::Core5Compat
)

add_executable(texteditor WIN32
    src/main.cpp
)
target_link_libraries(texteditor PRIVATE texteditor_lib)

include(FetchContent)
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.5.4
)
FetchContent_MakeAvailable(Catch2)

enable_testing()
add_subdirectory(tests)
