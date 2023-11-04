cmake_minimum_required(VERSION 3.25)
project(CG_Raycasting)

set(CMAKE_CXX_STANDARD 23)
set(SDL_INCLUDE_DIR C:/SDL/include)
set(SDL2_LIB_DIR C:/SDL/lib/x64)
set(SDL2_IMAGE_DIR C:/Users/mague/CLionProjects/CG_Raycasting/SDL2_image-2.6.3/include)
set(SDL2_IMAGE_LIB C:/Users/mague/CLionProjects/CG_Raycasting/SDL2_image-2.6.3/lib/x64/SDL2_image.dll)

include_directories("C:/MinGW/include")
include_directories(${SDL_INCLUDE_DIR})
include_directories(${SDL2_IMAGE_DIR})
link_directories(${SDL2_LIB_DIR})
link_directories(${SDL2_IMAGE_LIB})

add_executable(CG_Raycasting src/main.cpp include/constants.h include/FastNoiseLite.h
        include/isDefined.h include/print.h src/color.h src/imageLoader.h src/raycaster.h)

target_link_libraries(CG_Raycasting SDL2main SDL2 ${SDL2_IMAGE_LIB})