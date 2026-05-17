# Сборка и запуск на Windows

## Требования

- [Qt 6](https://www.qt.io/download-qt-installer) — при установке выбрать компонент **MinGW** (например, `Qt 6.x.x → MinGW 64-bit`)
- MinGW идёт в комплекте с Qt, устанавливается вместе с ним

## Команда для сборки проекта
```
$cmake = "C:\Qt\Tools\CMake_64\bin\cmake.exe"
$mingw = "C:\Qt\Tools\mingw1310_64\bin"
$qt = "C:\Qt\6.11.1\mingw_64"
$windeployqt = "C:\Qt\6.11.1\mingw_64\bin\windeployqt.exe"

& $cmake -S . -B build-cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release `
   -DCMAKE_MAKE_PROGRAM="$mingw\mingw32-make.exe" `
   -DCMAKE_CXX_COMPILER="$mingw\g++.exe" `
   -DCMAKE_PREFIX_PATH="$qt"
 
& $cmake --build build-cmake -j 4

& $windeployqt ./build-cmake/texteditor.exe
```

## Команда для запуска проекта
start .\build-cmake\texteditor.exe