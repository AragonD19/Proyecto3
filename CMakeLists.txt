cmake_minimum_required(VERSION 3.23)
project(Proyecto3)

set(CMAKE_CXX_STANDARD 20)

add_executable(Proyecto3 main.cpp camera.cpp camera.h sphere.cpp sphere.h color.h intersect.h light.h material.h object.h print.h cube.cpp cube.h skybox.cpp skybox.h)

target_link_libraries(${PROJECT_NAME} -lopengl32 -lfreeglut)

# --- SDL2 SETUP ---
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
set(SDL2_PATH "SDL2/x86_64-w64-mingw32")
set(SDL2_IMAGE_PATH "SDL2/x86_64-w64-mingw32")

find_package(SDL2_image REQUIRED)
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} ${SDL2_LIBRARY} ${SDL2_IMAGE_LIBRARY})