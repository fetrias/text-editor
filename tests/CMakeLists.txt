set(TEST_RUNTIME_DIR ${CMAKE_BINARY_DIR}/tests)

set(QT_BIN_DIR "")
set(QT_PLUGINS_DIR "")
if (DEFINED Qt6_DIR)
    get_filename_component(_qt_cmake_dir "${Qt6_DIR}" ABSOLUTE)
    get_filename_component(_qt_prefix "${_qt_cmake_dir}/../../.." ABSOLUTE)
    set(QT_BIN_DIR "${_qt_prefix}/bin")
    set(QT_PLUGINS_DIR "${_qt_prefix}/plugins")
elseif (CMAKE_PREFIX_PATH)
    list(GET CMAKE_PREFIX_PATH 0 _qt_prefix)
    set(QT_BIN_DIR "${_qt_prefix}/bin")
    set(QT_PLUGINS_DIR "${_qt_prefix}/plugins")
endif()

get_filename_component(MINGW_BIN_DIR "${CMAKE_CXX_COMPILER}" DIRECTORY)

set(WINDEPLOYQT_EXECUTABLE "")
if (WIN32 AND QT_BIN_DIR)
    if (EXISTS "${QT_BIN_DIR}/windeployqt.exe")
        set(WINDEPLOYQT_EXECUTABLE "${QT_BIN_DIR}/windeployqt.exe")
    else()
        find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${QT_BIN_DIR}")
    endif()
endif()

function(add_catch_executable target)
    add_executable(${target}
        ${CMAKE_CURRENT_LIST_DIR}/test_main.cpp
        ${ARGN}
    )
    target_include_directories(${target}
        PRIVATE
            ${PROJECT_SOURCE_DIR}/include
            ${CMAKE_CURRENT_LIST_DIR}
    )
    target_link_libraries(${target}
        PRIVATE
            texteditor_lib
            Catch2::Catch2
            Qt6::Core
            Qt6::Widgets
            Qt6::Test
            Qt6::Core5Compat
    )
    target_compile_definitions(${target} PRIVATE PKS_TESTING=1)
    set_target_properties(${target} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${TEST_RUNTIME_DIR}
    )
    add_test(NAME ${target} COMMAND ${target})

    if (WINDEPLOYQT_EXECUTABLE)
        add_custom_command(TARGET ${target} POST_BUILD
            COMMAND "${WINDEPLOYQT_EXECUTABLE}" "$<TARGET_FILE:${target}>" --compiler-runtime --no-translations
            COMMENT "Deploying Qt runtime for ${target}"
        )
    endif()

    if (WIN32)
        set(_target_dir "$<TARGET_FILE_DIR:${target}>")

        set(_qt_dlls
            Qt6Core.dll
            Qt6Gui.dll
            Qt6Widgets.dll
            Qt6Test.dll
            Qt6Core5Compat.dll
        )
        foreach(_dll IN LISTS _qt_dlls)
            if (EXISTS "${QT_BIN_DIR}/${_dll}")
                add_custom_command(TARGET ${target} POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
                        "${QT_BIN_DIR}/${_dll}" "${_target_dir}"
                )
            endif()
        endforeach()

        set(_mingw_dlls
            libstdc++-6.dll
            libgcc_s_seh-1.dll
            libwinpthread-1.dll
        )
        foreach(_dll IN LISTS _mingw_dlls)
            if (EXISTS "${MINGW_BIN_DIR}/${_dll}")
                add_custom_command(TARGET ${target} POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
                        "${MINGW_BIN_DIR}/${_dll}" "${_target_dir}"
                )
            endif()
        endforeach()

        if (EXISTS "${QT_PLUGINS_DIR}/platforms")
            add_custom_command(TARGET ${target} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E make_directory
                    "${_target_dir}/platforms"
            )
            foreach(_plugin IN ITEMS qwindows.dll qoffscreen.dll)
                if (EXISTS "${QT_PLUGINS_DIR}/platforms/${_plugin}")
                    add_custom_command(TARGET ${target} POST_BUILD
                        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                            "${QT_PLUGINS_DIR}/platforms/${_plugin}"
                            "${_target_dir}/platforms"
                    )
                endif()
            endforeach()
        endif()
    endif()

    set(_path_entries "")
    if (EXISTS "${QT_BIN_DIR}")
        file(TO_CMAKE_PATH "${QT_BIN_DIR}" _qt_bin_dir)
        list(APPEND _path_entries "${_qt_bin_dir}")
    endif()
    if (EXISTS "${MINGW_BIN_DIR}")
        file(TO_CMAKE_PATH "${MINGW_BIN_DIR}" _mingw_bin_dir)
        list(APPEND _path_entries "${_mingw_bin_dir}")
    endif()
    file(TO_CMAKE_PATH "$ENV{PATH}" _env_path)
    list(APPEND _path_entries "${_env_path}")
    list(JOIN _path_entries ";" _path_value)

    set(_env_list "PATH=${_path_value}" "QT_QPA_PLATFORM=offscreen")
    if (EXISTS "${QT_PLUGINS_DIR}")
        file(TO_CMAKE_PATH "${QT_PLUGINS_DIR}" _qt_plugins_dir)
        list(APPEND _env_list "QT_PLUGIN_PATH=${_qt_plugins_dir}")
    endif()
    set_tests_properties(${target} PROPERTIES ENVIRONMENT ${_env_list})
endfunction()

add_catch_executable(unit_codeeditor unit/test_codeeditor.cpp)
add_catch_executable(unit_syntaxhighlighter unit/test_syntaxhighlighter.cpp)
add_catch_executable(unit_pomodorotimer unit/test_pomodorotimer.cpp)
add_catch_executable(unit_mainwindow unit/test_mainwindow.cpp)

add_catch_executable(scenario_basic_edit scenarios/scenario_basic_edit.cpp)
add_catch_executable(scenario_open_find_save scenarios/scenario_open_find_save.cpp)
add_catch_executable(scenario_pomodoro_flow scenarios/scenario_pomodoro_flow.cpp)
