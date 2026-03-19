cmake_minimum_required(VERSION 3.20)

project(Nionyx LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(TRACY_ENABLE "" ON)

add_library(glad STATIC src/external/glad.c)
set_target_properties(glad PROPERTIES LINKER_LANGUAGE C)
add_library(imgui STATIC
    src/external/imgui.cpp
    src/external/imgui_demo.cpp
    src/external/imgui_draw.cpp
    src/external/imgui_impl_glfw.cpp
    src/external/imgui_impl_opengl3.cpp
    src/external/imgui_stdlib.cpp
    src/external/imgui_tables.cpp
    src/external/imgui_widgets.cpp
)

target_include_directories(imgui PUBLIC
    include
)

target_include_directories(glad PUBLIC
    include
)

find_package(OpenGL REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(assimp CONFIG REQUIRED)
find_package(lz4 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(Tracy CONFIG REQUIRED)

target_link_libraries(imgui PUBLIC
	glfw
)

add_executable(Nionyx
    src/main.cpp
    src/Loader.cpp
    src/gui.cpp
    src/physics.cpp
    src/game.cpp
    src/NXPK/NXPKLoader.cpp
)

target_include_directories(Nionyx PRIVATE
    include
    custominclude
)

target_link_libraries(Nionyx PRIVATE
    OpenGL::GL
    glfw
    assimp::assimp
    lz4::lz4
    glm::glm
    Tracy::TracyClient

    glad
    imgui
)

if (APPLE)
    target_link_libraries(Nionyx PRIVATE
        "-framework Cocoa"
        "-framework IOKit"
        "-framework CoreVideo"
    )
elseif (UNIX AND NOT APPLE)
    target_link_libraries(Nionyx PRIVATE
        pthread
        dl
    )
endif()

target_precompile_headers(Nionyx PRIVATE
    custominclude/pch.hpp
    custominclude/camera.hpp
    custominclude/NXPK.hpp
    custominclude/shader.hpp
    custominclude/camera.hpp
)
