cmake_minimum_required(VERSION 3.25)
project(CG_Raycasting)

set(CMAKE_CXX_STANDARD 23)
set(SDL_INCLUDE_DIR C:/SDL/include)
set(SDL2_LIB_DIR C:/SDL/lib/x64)

include_directories(${SDL_INCLUDE_DIR})
link_directories(${SDL2_LIB_DIR})
include_directories("C:/MinGW/include")

add_executable(CG_Raycasting src/main.cpp)

target_link_libraries(CG_Raycasting SDL2main SDL2)