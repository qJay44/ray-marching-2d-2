include(./.env.cmake OPTIONAL RESULT_VARIABLE LOCAL_ENV)
message(STATUS "Local .env.cmake: ${LOCAL_ENV}")

cmake_minimum_required(VERSION 3.21)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Run)

project(MyProject VERSION 1.0 LANGUAGES C CXX)

find_package(OpenGL REQUIRED)
find_package(SFML 3 REQUIRED COMPONENTS System Window Graphics)

# ===== GLAD ====================================================== #

add_library(GLAD STATIC ${GLAD_PATH}/src/gl.c)
target_include_directories(GLAD PUBLIC ${GLAD_PATH}/include)

# ===== ImGui (SFML) ============================================== #

add_library(ImGui_SFML STATIC
  ${IMGUI_PATH}/imgui.cpp
  ${IMGUI_PATH}/imgui_draw.cpp
  ${IMGUI_PATH}/imgui_tables.cpp
  ${IMGUI_PATH}/imgui_widgets.cpp
  ${IMGUI_SFML_PATH}/imgui-SFML.cpp
)

target_include_directories(ImGui_SFML PUBLIC ${IMGUI_PATH} ${IMGUI_SFML_PATH})
target_link_libraries(ImGui_SFML PRIVATE GLAD)

# ===== My utils ================================================== #

file(GLOB_RECURSE UTILS_SOURCES ${UTILS_PATH}/src/*.cpp)

# Remove TIFF from sources
list(REMOVE_ITEM UTILS_SOURCES ${UTILS_PATH}/src/loadTif.cpp)
list(REMOVE_ITEM UTILS_SOURCES ${UTILS_PATH}/src/saveTif.cpp)

add_library(Utils STATIC ${UTILS_SOURCES})

target_include_directories(Utils PUBLIC ${UTILS_PATH}/include)

# ===== Main ====================================================== #

file(GLOB_RECURSE SOURCES ${PROJECT_SOURCE_DIR}/src/*.cpp)
add_executable(${PROJECT_NAME} ${SOURCES})

target_compile_options(${PROJECT_NAME} PRIVATE -Wall)

target_include_directories(${PROJECT_NAME} PRIVATE
  ${GLAD_PATH}/include
  ${UTILS_PATH}/include
  ${PROFILER_PATH}/include
  ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(${PROJECT_NAME} PRIVATE
  SFML::System
  SFML::Window
  SFML::Graphics
  GLAD
  Utils
  ImGui_SFML
  OpenGL::GL
)

target_precompile_headers(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/src/pch.hpp)

