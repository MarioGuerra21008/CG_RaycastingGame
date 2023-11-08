#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_events.h>
#include <SDL_render.h>
#include <SDL_audio.h>
#include <SDL_video.h>
#include <chrono>
#include "imageloader.h"
#include "raycaster.h"

SDL_Window* window;
SDL_Renderer* renderer;
bool hasWon = false;

typedef struct {
    uint8_t* start;        // Puntero al inicio del archivo de audio
    uint8_t* pos;
    uint32_t length;
    uint32_t totalLength;  // Longitud total del archivo de audio
} AudioData;

void AudioCallback(void* userData, Uint8* stream, int len){
    if(len > 0){
        AudioData* audio = (AudioData*)userData;

        if (audio->length == 0){
            // Reiniciar la reproducción desde el principio
            audio->pos = audio->start;
            audio->length = audio->totalLength;
        }

        len = (len > audio->length ? audio->length : len);

        SDL_memcpy(stream, audio->pos, len);

        audio->pos += len;
        audio->length -= len;
    }
}

void clear(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 56, 56, 56, 255);
    SDL_RenderClear(renderer);
}

void welcomeScreen(SDL_Renderer* renderer) {
    bool welcome = false;
    while (!welcome) {
        ImageLoader::render(renderer, "welcome_image", 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
        SDL_RenderPresent(renderer);
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                welcome = true;
            } else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_SPACE) {
                    welcome = true; // Sal del bucle al presionar la barra espaciadora
                }
            }
        }
    }
}

void draw_floor(SDL_Renderer* renderer, Color floorColor, Color backgroundColor){
    SDL_SetRenderDrawColor(renderer, backgroundColor.r, backgroundColor.g, backgroundColor.b, backgroundColor.a);
    SDL_Rect backgroundRect = {
            0,
            0,
            SCREEN_WIDTH,
            SCREEN_HEIGHT
    };
    SDL_RenderFillRect(renderer, &backgroundRect);
    SDL_SetRenderDrawColor(renderer, floorColor.r, floorColor.g, floorColor.b, floorColor.a);
    SDL_Rect floorRect = {
            0,
            SCREEN_HEIGHT / 2,
            SCREEN_WIDTH,
            SCREEN_HEIGHT / 2
    };
    SDL_RenderFillRect(renderer, &floorRect);
}

void loadMapAndRunGame(Raycaster& r, const std::string& mapFilePath) {
    r.load_map(mapFilePath);
    bool running = true;

    int frameCounter = 0;
    double totalTime = 0.0;
    int fps = 0;
    auto startTime = std::chrono::high_resolution_clock::now();

    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
                break;
            }
            if (event.type == SDL_KEYDOWN) {
                switch(event.key.keysym.sym) {
                    case SDLK_LEFT:
                        r.player.a += 3.14 / 24;
                        break;
                    case SDLK_RIGHT:
                        r.player.a -= 3.14 / 24;
                        break;
                    case SDLK_UP:
                        r.sdlk_up();
                        break;
                    case SDLK_DOWN:
                        r.sdlk_down();
                        break;
                    default:
                        break;
                }
            }
        }
        clear(renderer);
        draw_floor(renderer, floorColor, myBackgroundColor);
        r.render();
        SDL_RenderPresent(renderer);

        auto endTime = std::chrono::high_resolution_clock::now();
        double frameTime = std::chrono::duration<double>(endTime - startTime).count();
        startTime = endTime;

        totalTime += frameTime;
        frameCounter++;

        if (totalTime >= 1.0) {
            fps = static_cast<int>(frameCounter);
            frameCounter = 0;
            totalTime = 0.0;
            std::string windowTitle = "SMT: Persona Labyrinth | FPS: " + std::to_string(fps);
            SDL_SetWindowTitle(window, windowTitle.c_str());
        }

        /*if (hasWon){
            r.draw_victory_screen();
            running = false;
        } */
    }
}

int main() {
    SDL_Init(SDL_INIT_VIDEO);
    ImageLoader::init();

    window = SDL_CreateWindow("SMT: Persona Labyrinth", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    ImageLoader::loadImage("+", "../assets/wall1.png");
    ImageLoader::loadImage("-", "../assets/wall2.png");
    ImageLoader::loadImage("|", "../assets/wall3.png");
    ImageLoader::loadImage("*", "../assets/wall4.png");
    ImageLoader::loadImage("g", "../assets/exit.png");
    ImageLoader::loadImage("welcome_image", "../assets/welcome.jpg");
    //ImageLoader::loadImage("win", "../assets/victory.jpg");
    ImageLoader::loadImage("level_image", "../assets/levelselector.png");

    Raycaster r = { renderer };

    SDL_AudioSpec wavSpec;
    Uint8* wavStart;
    Uint32 wavLength;

    if (SDL_LoadWAV("../assets/SchoolDaysPersona1.wav", &wavSpec, &wavStart, &wavLength) == NULL) {
        SDL_Log("Error al cargar el archivo de audio: %s\n", SDL_GetError());
        return 1;
    }

    AudioData audioData;
    audioData.start = wavStart;
    audioData.pos = wavStart;
    audioData.length = wavLength;
    audioData.totalLength = wavLength;

    wavSpec.callback = AudioCallback;
    wavSpec.userdata = &audioData;

    if (SDL_OpenAudio(&wavSpec, NULL) < 0) {
        SDL_Log("No se pudo abrir el dispositivo de audio: %s\n", SDL_GetError());
        return 1;
    }

    SDL_PauseAudio(0);

    bool running = true;
    welcomeScreen(renderer);
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImageLoader::render(renderer, "level_image", 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
            SDL_RenderPresent(renderer);
            if (event.type == SDL_QUIT) {
                running = false;
                break;
            }
            if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                    case SDLK_1:
                        loadMapAndRunGame(r, "../assets/level1.txt");
                        break;
                    case SDLK_2:
                        loadMapAndRunGame(r, "../assets/level2.txt");
                        break;
                    case SDLK_3:
                        loadMapAndRunGame(r, "../assets/level3.txt");
                        break;
                }
            }
        }
    }
    SDL_CloseAudio();
    SDL_FreeWAV(wavStart);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}