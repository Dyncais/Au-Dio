//
// Created by Dima on 24.09.2023.
//
#include "SDL3/SDL_render.h"
#include "string"
#include <SDL3/SDL.h>
#include <cstdint>
#include <functional>
#include <string>


#pragma once

class IWINDOW {

  SDL_Renderer *render;

  bool is_running = true;

  std::function<void(const std::string &)> m_OnDragAndDrop;

public:
  SDL_Window *window; // изменить на get
  void BeginDrawing();
  void EndDrawing(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
  IWINDOW(int width, int height, std::string Name);
  ~IWINDOW();

  void SetFunctionThatCallsOnDragAndDrop(
      const std::function<void(const std::string &)> &func) {
    m_OnDragAndDrop = func;
  }

  SDL_Renderer* Rendered(){return render;}
  bool IsRunning() const { return is_running; }

  SDL_Window *GetWindow() const { return window; }

  void Events();

  void StartFrame();
};
