cmake_minimum_required(VERSION 3.15)

project(simple_yuv_viewer)

find_package(SDL2 REQUIRED)
add_subdirectory(libyuv EXCLUDE_FROM_ALL)
target_include_directories(yuv PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/libyuv/include)

find_package ( JPEG )
if (JPEG_FOUND)
    include_directories( ${JPEG_INCLUDE_DIR} )
    target_link_libraries( yuv PUBLIC ${JPEG_LIBRARY} )
endif()

if (ENABLE_ASAN)
    message(STATUS "build with ASAN")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
endif ()

set(imgui_src_dir "${CMAKE_CURRENT_SOURCE_DIR}/imgui")
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
target_link_libraries(yuv_viewer PRIVATE imgui_sdl2_impl yuv)