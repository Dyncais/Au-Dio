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

void audio_callback(void* userdata, Uint8* stream, int len);

const int BUFFER_SIZE = 2048;
float audio_buffer[BUFFER_SIZE];

void audio_callback(void* userdata, Uint8* stream, int len) {
    int num_samples = len / sizeof(float);
    float* samples = reinterpret_cast<float*>(stream);

    for (int i = 0; i < num_samples; ++i) {
        audio_buffer[i] = samples[i];
    }

    // Пример использования FFTW
    fftw_complex* in = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * num_samples);
    fftw_complex* out = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * num_samples);
    fftw_plan p = fftw_plan_dft_1d(num_samples, in, out, FFTW_FORWARD, FFTW_ESTIMATE);

    for (int i = 0; i < num_samples; ++i) {
        in[i][0] = audio_buffer[i];  // Реальная часть
        in[i][1] = 0.0;              // Мнимая часть
    }

    fftw_execute(p);

    // Обработка результатов FFT
    for (int i = 0; i < num_samples / 2; ++i) {
        float magnitude = sqrt(out[i][0] * out[i][0] + out[i][1] * out[i][1]);
        // Выводим или сохраняем данные для визуализации
        std::cout << "Magnitude[" << i << "]: " << magnitude << std::endl;
    }

    fftw_destroy_plan(p);
    fftw_free(in);
    fftw_free(out);
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

    Mix_Music* music = Mix_LoadMUS( "C:\\Users\\Dima\\Desktop\\mus_vsufsans.wav");

    Mix_PlayMusic(music, -1);

    while (iwindow.IsRunning()) {
        iwindow.Events();
        iwindow.StartFrame();

        iwindow.BeginDrawing();


        Mix_SetPostMix(audio_callback, nullptr);

        iwindow.EndDrawing(cl.r * 255, cl.g * 255, cl.b * 255, 255);
    }

    // Cleanup
    Mix_CloseAudio();
    SDL_Quit();

    return 0;
}



        // SDL_Renderer* renderer = SDL_GetRenderer(iwindow.GetWindow());
        // if (renderer) {
        //     // Проверка простого рендеринга линии
        //     SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        //     SDL_RenderLine(renderer, 0, 0, 762, 540);
            
        // }