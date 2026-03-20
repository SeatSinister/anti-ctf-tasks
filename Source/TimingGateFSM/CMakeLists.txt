cmake_minimum_required(VERSION 3.20)
project(TimingGateFSM LANGUAGES CXX)

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

add_executable(timing_gate WIN32
  main.cpp
  ${imgui_SOURCE_DIR}/imgui.cpp
  ${imgui_SOURCE_DIR}/imgui_draw.cpp
  ${imgui_SOURCE_DIR}/imgui_tables.cpp
  ${imgui_SOURCE_DIR}/imgui_widgets.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

target_include_directories(timing_gate PRIVATE
  ${imgui_SOURCE_DIR}
  ${imgui_SOURCE_DIR}/backends
)

target_link_libraries(timing_gate PRIVATE glfw OpenGL::GL lunasvg)
if(WIN32)
  target_link_libraries(timing_gate PRIVATE opengl32 user32 gdi32 shell32)
endif()

if(MSVC)
  target_compile_options(timing_gate PRIVATE /utf-8)
  target_compile_options(timing_gate PRIVATE "$<$<CONFIG:Release>:/O2>")
  target_link_options(timing_gate PRIVATE "$<$<CONFIG:Release>:/DEBUG:NONE>")
else()
  target_compile_options(timing_gate PRIVATE "$<$<CONFIG:Release>:-O2>")
  target_link_options(timing_gate PRIVATE "$<$<CONFIG:Release>:-s>")
endif()
