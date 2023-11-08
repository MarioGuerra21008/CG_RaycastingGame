#pragma once
#define SDL_MAIN_HANDLED
#include <iostream>
#include <fstream>
#include <sstream>
#include <SDL_render.h>
#include <string>
#include <vector>
#include <cmath>
#include <SDL.h>
#include <unordered_map>
#include "color.h"
#include "imageLoader.h"

const Color B = {0, 0, 0};
const Color W = {255, 255, 255};
const Color myBackgroundColor = {154, 166, 161};
const Color floorColor = {67, 77, 73};
const Color mapColor = {67, 77, 73};

const int WIDTH = 16;
const int HEIGHT = 11;
const int BLOCK = 50;

const int SCREEN_WIDTH = WIDTH * BLOCK;
const int SCREEN_HEIGHT = HEIGHT * BLOCK;

const int blockSize = 17;
const int mapWidth = blockSize * WIDTH;
const int mapHeight = blockSize * HEIGHT;

struct Player {
    int x;
    int y;
    float a;
    float fov;
};

struct Impact {
    float d;
    std::string mapHit;  // + | -
    int tx;
};

class Raycaster {
public:
    Raycaster(SDL_Renderer* renderer) : renderer(renderer) {

        player.x = blockSize + blockSize / 2;
        player.y = blockSize + blockSize / 2;

        player.a = M_PI / 4.0f;
        player.fov = M_PI / 3.0f;

        scale = 50;
        tsize = 128;
    }

    void sdlk_up(){
        int speed = 10;
        int newX = static_cast<int>(player.x + speed * cos(player.a));
        int newY = static_cast<int>(player.y + speed * sin(player.a));

        int newI = newX / blockSize;
        int newJ = newY / blockSize;

        if (map[newJ][newI] == ' ') {
            player.x = newX;
            player.y = newY;
        }
    }

    void sdlk_down(){
        int speed = 10;
        // Calcula la nueva posición después de moverse hacia adelante
        int newX = static_cast<int>(player.x - speed * cos(player.a));
        int newY = static_cast<int>(player.y - speed * sin(player.a));

        // Convierte la nueva posición en índices de matriz
        int newI = newX / blockSize;
        int newJ = newY / blockSize;

        // Verifica si la nueva posición es un espacio en blanco en el mapa
        if (map[newJ][newI] == ' ') {
            // Actualiza la posición del jugador
            player.x = newX;
            player.y = newY;
        }
    }

    void load_map(const std::string& filename) {
        std::ifstream file(filename);
        std::string line;
        while (getline(file, line)) {
            map.push_back(line);
        }
        file.close();
    }

    void point(int x, int y, Color c) {
        SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);
        SDL_RenderDrawPoint(renderer, x, y);
    }

    void rect(int x, int y, const std::string& mapHit) {
        for(int cx = x; cx < x + blockSize; cx++) {
            for(int cy = y; cy < y + blockSize; cy++) {
                int tx = ((cx - x) * tsize) / BLOCK;
                int ty = ((cy - y) * tsize) / BLOCK;

                Color c = ImageLoader::getPixelColor(mapHit, tx, ty);
                SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b , 255);
                SDL_RenderDrawPoint(renderer, cx, cy);
            }
        }
    }

    Impact cast_ray(float a) {
        float d = 0;
        std::string mapHit;
        int tx;

        while(true) {
            int x = static_cast<int>(player.x + d * cos(a));
            int y = static_cast<int>(player.y + d * sin(a));

            int i = static_cast<int>(x / blockSize);
            int j = static_cast<int>(y / blockSize);


            if (map[j][i] != ' ') {
                mapHit = map[j][i];

                int hitx = x - i * blockSize;
                int hity = y - j * blockSize;
                int maxhit;

                if (hitx == 0 || hitx == BLOCK - 1) {
                    maxhit = hity;
                } else {
                    maxhit = hitx;
                }
                tx = maxhit * tsize / BLOCK;
                break;
            }
            point(x, y, W);
            d += 1;
        }
        return Impact{d, mapHit, tx};
    }

    void draw_stake(int x, float h, Impact i) {
        float start = SCREEN_HEIGHT/2.0f - h/2.0f;
        float end = start + h;

        for (int y = start; y < end; y++) {
            int ty = (y - start) * tsize / h;
            Color c = ImageLoader::getPixelColor(i.mapHit, i.tx, ty);
            SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);
            SDL_RenderDrawPoint(renderer, x, y);
        }
    }
/*
    bool has_won() {
        int player_x = static_cast<int>(player.x / BLOCK);
        int player_y = static_cast<int>(player.y / BLOCK);

        if (map[player_y][player_x] == 'g') {
            return true;
        }
        return false;
    }

    void draw_victory_screen() {
        bool hasWon = false;
        if(has_won()){
            hasWon = true;
            SDL_RenderClear(renderer);
            // Dibuja la pantalla de victoria (imagen o texto)
            ImageLoader::render(renderer, "win", 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
            // Refresca el renderizador
            SDL_RenderPresent(renderer);
        }
    }
*/
    void render() {
        // draw left side of the screen
        for (int i = 1; i < SCREEN_WIDTH; i++) {
            double a = player.a + player.fov / 2.0 - player.fov * i / SCREEN_WIDTH;
            Impact impact = cast_ray(a);
            float d = impact.d;

            if (d == 0) {
                std::cout << "you lose" << std::endl;
                exit(1);
            }

            int x = i;
            float h = static_cast<float>(SCREEN_HEIGHT)/static_cast<float>(d) * static_cast<float>(scale);
            draw_stake(x, h, impact);
        }


        for (int x = 0; x < mapWidth; x += blockSize) {
            for (int y = 0; y < mapHeight; y += blockSize) {
                int i = static_cast<int>(x / blockSize);
                int j = static_cast<int>(y / blockSize);

                if (map[j][i] != ' ') {
                    std::string mapHit;
                    mapHit = map[j][i];
                    Color c = Color(255, 0, 0);
                    rect(x, y, mapHit);
                } else{
                    SDL_SetRenderDrawColor(renderer, mapColor.r, mapColor.g, mapColor.b, mapColor.a);
                    SDL_Rect rect = {x, y, blockSize, blockSize};
                    SDL_RenderFillRect(renderer, &rect);
                }
            }
        }

        for (int i = 0; i < mapWidth; i++) {
            float a = player.a + player.fov / 2 - player.fov * i / mapWidth;
            cast_ray(a);
        }
    }
    Player player;
private:
    int scale;
    SDL_Renderer* renderer;
    std::vector<std::string> map;
    int tsize;
};