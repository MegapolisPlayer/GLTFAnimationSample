cmake_minimum_required(VERSION 3.12)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

add_compile_options(-Wall -Wextra -g -g3 -fno-omit-frame-pointer -fsanitize=address -Wno-unused-parameter -Wno-reorder-ctor) # disable for release mode

project(gl3d VERSION 0.1.0)
add_compile_definitions(GLEW_NO_GLU)
add_executable(gl3d
"GL3D.cpp"
"Mesh.cpp"
"Model.cpp"
"Animation.cpp"
"Shader.cpp"
"Texture.cpp"

"depend/glad/src/glad.c"

"depend/imgui/imgui.cpp"
"depend/imgui/imgui_tables.cpp"
"depend/imgui/imgui_widgets.cpp"
"depend/imgui/imgui_draw.cpp"
"depend/imgui/imgui_draw.cpp"
"depend/imgui/backends/imgui_impl_opengl3.cpp"
"depend/imgui/backends/imgui_impl_glfw.cpp"
"depend/imgui/misc/cpp/imgui_stdlib.cpp"

"depend/fastgltf/base64.cpp"
"depend/fastgltf/fastgltf.cpp"
"depend/fastgltf/io.cpp"
"depend/fastgltf/simdjson.cpp"
)
target_include_directories(gl3d PUBLIC
"depend/"
"depend/imgui/"
"depend/imgui/backends/"
"depend/imgui/misc/cpp/"
"depend/fastgltf/"
"depend/glad/include/"
)
target_link_directories(gl3d PUBLIC "depend/")
target_link_libraries(gl3d PUBLIC -lGL -lglfw -fsanitize=address)






