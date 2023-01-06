cmake_minimum_required(VERSION 3.15)

project(simple_yuv_viewer)

find_package(SDL2 REQUIRED)

set(imgui_src_dir "/Users/user/Documents/develop/imgui")
file(GLOB imgui_src_files ${imgui_src_dir}/*.cpp)
add_library(imgui_sdl2_impl
        ${imgui_src_files}
        ${imgui_src_dir}/backends/imgui_impl_sdl.cpp
        ${imgui_src_dir}/backends/imgui_impl_sdlrenderer.cpp
        )
target_include_directories(imgui_sdl2_impl PUBLIC
        ${imgui_src_dir}
        ${imgui_src_dir}/backends
        ${SDL2_INCLUDE_DIRS})
target_compile_features(imgui_sdl2_impl PUBLIC cxx_std_17)
target_link_libraries(imgui_sdl2_impl PUBLIC ${SDL2_LIBRARIES} "-framework OpenGl")

add_executable(yuv_viewer main.cpp)
target_link_libraries(yuv_viewer PRIVATE imgui_sdl2_impl)