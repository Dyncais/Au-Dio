//
// Created by Dima on 24.09.2023.
//

#include "IWINDOW.h"
#include <iostream>
#include <SDL3/SDL.h>
#include <imgui.h>
#include <backends/imgui_impl_sdl3.h>
#include <backends/imgui_impl_sdlrenderer3.h>
#include "OpenSans_Regular_ttf.h"
#include "SDL3_mixer/SDL_mixer.h"

void IWINDOW::Drawing(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    ImGui::Render();

    if(SDL_SetRenderDrawColor(render,r,g,b,a))
    {
        std::cout << "Error: " << SDL_GetError() << '\n';
    }

    SDL_RenderClear(render);

    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData());

    SDL_RenderPresent(render);
}

IWINDOW::IWINDOW(int width, int height, std::string Name)
{
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    window = SDL_CreateWindow(Name.c_str(), width,height,SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    render = SDL_CreateRenderer(window, "opengl",SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    //Initialize ImGui
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    const ImWchar iconsRanges[] = {0x20, 0xFFFF, 0};
    ImFontConfig config {};

    io.FontDefault = io.Fonts->AddFontFromMemoryTTF(OpenSans_Regular_ttf_data, OpenSans_Regular_ttf_size, 18.0f, &config, iconsRanges);
    io.Fonts->Build();

    ImGui_ImplSDL3_InitForSDLRenderer(window, render);
    ImGui_ImplSDLRenderer3_Init(render);
}

IWINDOW::~IWINDOW()
{

    Mix_CloseAudio();
    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    //ImGui::DestroyContext();

    SDL_DestroyRenderer(render);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void IWINDOW::Events()
{
    SDL_Event NY;
    while (SDL_PollEvent(&NY))
    {
        ImGui_ImplSDL3_ProcessEvent(&NY);
        if (NY.type == SDL_EVENT_QUIT)
        {
            is_running = false;
        }

        if (NY.type == SDL_EVENT_DROP_FILE) {
            char* dropped_filedir = NY.drop.data;
            m_OnDragAndDrop(dropped_filedir);

        }
    };
}

void IWINDOW::StartFrame()
{
    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

}


