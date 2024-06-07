#include <iostream>
#include "IWINDOW.h"
#include "SDL3_mixer/SDL_mixer.h"
#include "SDL_oldnames.h"
#include "sndfile.h"
#include "imgui.h"
#include <filesystem>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <cstdint>
#include <SDL3/SDL.h>
#include <fftw3.h>

struct COLORS {
    float r = 1, g = 1, b = 1;
};

uint64_t factorial(uint64_t n)
{
    static std::unordered_map<uint64_t, uint64_t> cache {{0,1}};
    if(cache.contains(n))
    {
        return cache.at(n);
    }
    return n * factorial(n-1);
}


int main() {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        std::cerr << "Failed to initialize SDL: " << SDL_GetError() << std::endl;
        return 1;
    }

    COLORS cl;
    IWINDOW iwindow(762, 540, "");
    
    SDL_AudioSpec spec;
    spec.channels = 2;
    spec.format = MIX_DEFAULT_FORMAT;
    spec.freq = MIX_DEFAULT_FREQUENCY;
    if (Mix_OpenAudio(0, &spec) < 0) {
        std::cerr << "Ошибка инициализации SDL_mixer: " << Mix_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    const std::string filename = "C:\\Users\\Dima\\Desktop\\mus_vsufsans.wav";
    //auto audioData = getAudioData(filename);

    while (iwindow.IsRunning()) {
        iwindow.Events();
        iwindow.StartFrame();

        iwindow.BeginDrawing();
        SDL_Renderer* renderer = SDL_GetRenderer(iwindow.GetWindow());
        if (renderer) {
            // Проверка простого рендеринга линии
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
            SDL_RenderLine(renderer, 0, 0, 762, 540);
            
        }

        // Start ImGui frame

        //ImGui::Begin("Gayteka");
        //ImGui::End();
        iwindow.EndDrawing(cl.r * 255, cl.g * 255, cl.b * 255, 255);
    }

    // Cleanup
    Mix_CloseAudio();
    SDL_Quit();

    return 0;
}
