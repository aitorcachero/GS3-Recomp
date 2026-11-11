cmake_minimum_required(VERSION 3.20)
project(NintendoDSRecomp LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Prevenir advertencias de compatibilidad de MSVC
add_compile_definitions(
    _CRT_SECURE_NO_WARNINGS
    NOMINMAX
    WIN32_LEAN_AND_MEAN
)

# Rutas de inclusión
include_directories(
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${CMAKE_CURRENT_SOURCE_DIR}/third_party/SDL2/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/third_party/imgui"
    "${CMAKE_CURRENT_SOURCE_DIR}/third_party/imgui/backends"
)

# Fuentes
set(PROJECT_SOURCES
    src/main.cpp
    src/nds_config.cpp
    src/nds_rom_parser.cpp
    src/nds_modern_launcher.cpp
    src/nds_game_runner.cpp
    third_party/imgui/imgui.cpp
    third_party/imgui/imgui_draw.cpp
    third_party/imgui/imgui_tables.cpp
    third_party/imgui/imgui_widgets.cpp
    third_party/imgui/backends/imgui_impl_sdl2.cpp
    third_party/imgui/backends/imgui_impl_opengl3.cpp
)

add_executable(GoldenSun3_Recomp ${PROJECT_SOURCES})

# Bibliotecas
set(SDL2_LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_party/SDL2/lib/x64")

target_link_libraries(GoldenSun3_Recomp PRIVATE
    "${SDL2_LIB_DIR}/SDL2.lib"
    "${SDL2_LIB_DIR}/SDL2main.lib"
    opengl32
    comdlg32
    user32
    gdi32
    shell32
    ws2_32
)

# Propiedades del ejecutable (GUI puro sin ventana cmd en segundo plano)
set_target_properties(GoldenSun3_Recomp PROPERTIES
    WIN32_EXECUTABLE TRUE
    LINK_FLAGS "/ENTRY:mainCRTStartup"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_SOURCE_DIR}/build-debug"
)

# Copiar SDL2.dll junto al ejecutable si es necesario
add_custom_command(TARGET GoldenSun3_Recomp POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/SDL2.dll"
        "$<TARGET_FILE_DIR:GoldenSun3_Recomp>/SDL2.dll"
    VERBATIM
)
