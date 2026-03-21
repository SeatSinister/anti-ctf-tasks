cmake_minimum_required(VERSION 3.20)
project(ImGuiVaultFresh LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
  glfw
  GIT_REPOSITORY https://github.com/glfw/glfw.git
  GIT_TAG 3.4
)

FetchContent_Declare(
  imgui
  GIT_REPOSITORY https://github.com/ocornut/imgui.git
  GIT_TAG v1.92.6
)

FetchContent_Declare(
  lunasvg
  GIT_REPOSITORY https://github.com/sammycage/lunasvg.git
  GIT_TAG v3.0.1
)

FetchContent_MakeAvailable(glfw imgui lunasvg)
find_package(OpenGL REQUIRED)

add_executable(vault_fresh WIN32
  main.cpp
  ${imgui_SOURCE_DIR}/imgui.cpp
  ${imgui_SOURCE_DIR}/imgui_draw.cpp
  ${imgui_SOURCE_DIR}/imgui_tables.cpp
  ${imgui_SOURCE_DIR}/imgui_widgets.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

set(MIDNIGHT_SHARED_ASSETS "${CMAKE_CURRENT_SOURCE_DIR}/../SharedAssets")
target_include_directories(vault_fresh PRIVATE
  ${imgui_SOURCE_DIR}
  ${imgui_SOURCE_DIR}/backends
  ${MIDNIGHT_SHARED_ASSETS}
)

if(WIN32)
  set(_embed_font "${MIDNIGHT_SHARED_ASSETS}/Meltaface regular.ttf")
  set(_embed_svg "${MIDNIGHT_SHARED_ASSETS}/background_2.svg")
  if(NOT EXISTS "${_embed_font}")
    message(FATAL_ERROR "Embedded font missing (copy into SharedAssets): ${_embed_font}")
  endif()
  if(NOT EXISTS "${_embed_svg}")
    message(FATAL_ERROR "Embedded SVG missing (copy into SharedAssets): ${_embed_svg}")
  endif()
  set(EMBED_FONT_PATH "${_embed_font}")
  set(EMBED_SVG_PATH "${_embed_svg}")
  string(REPLACE "\\" "/" EMBED_FONT_PATH "${EMBED_FONT_PATH}")
  string(REPLACE "\\" "/" EMBED_SVG_PATH "${EMBED_SVG_PATH}")
  configure_file(
    "${MIDNIGHT_SHARED_ASSETS}/embed.rc.in"
    "${CMAKE_CURRENT_BINARY_DIR}/midnight_embed.rc"
    @ONLY
  )
  target_sources(vault_fresh PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/midnight_embed.rc")
endif()

target_link_libraries(vault_fresh PRIVATE glfw OpenGL::GL lunasvg)
if(WIN32)
  target_link_libraries(vault_fresh PRIVATE opengl32 user32 gdi32 shell32)
endif()

# Явно фиксируем UTF-8 для исходников, чтобы избежать кракозябр.
if(MSVC)
  target_compile_options(vault_fresh PRIVATE /utf-8)
  target_compile_options(vault_fresh PRIVATE "$<$<CONFIG:Release>:/O2>")
  target_link_options(vault_fresh PRIVATE "$<$<CONFIG:Release>:/DEBUG:NONE>")
else()
  target_compile_options(vault_fresh PRIVATE "$<$<CONFIG:Release>:-O2>")
  target_link_options(vault_fresh PRIVATE "$<$<CONFIG:Release>:-s>")
endif()

