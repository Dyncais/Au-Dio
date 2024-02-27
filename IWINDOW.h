//
// Created by Dima on 24.09.2023.
//
#include "string"
#include <cstdint>
#include <SDL3/SDL.h>
#include <functional>
#include <string>

#pragma once


class IWINDOW
{

    SDL_Renderer* render;

    bool is_running = true;

    std::function<void(const std::string&)> m_OnDragAndDrop;

public:
    SDL_Window* window; //изменить на get
    void Drawing(uint8_t r,uint8_t g,uint8_t b,uint8_t a);
    IWINDOW(int width,int height,std::string Name);
    ~IWINDOW();

    void SetFunctionThatCallsOnDragAndDrop(const std::function<void(const std::string&)>& func)
    {
        m_OnDragAndDrop = func;
    }

    bool IsRunning() const
    {
        return is_running;
    }

    void Events();

    void StartFrame();


};
