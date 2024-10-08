
#include "SDL3/SDL_iostream.h"
#include "SDL3/SDL_oldnames.h"
#include "SDL3/SDL_pixels.h"
#include "SDL3/SDL_surface.h"
#include <exception>

#define STB_IMAGE_IMPLEMENTATION
#include "IWINDOW.h"
#include "SDL3/SDL_opengl.h"
#include "SDL3_mixer/SDL_mixer.h"
#include "imgui.h"
#include "import.h"
#include "misc/cpp/imgui_stdlib.h"
#include <filesystem>
#include <iostream>

#include <SDL3/SDL.h>
#include <windows.h>
#include <winuser.h>
//#include "stb_image.h"

#include "DeletionQueue.h"
#include "ForWindows/Windows.h"
#include "ForWindows/json.hpp"
#include "ForWindows/tinyfiledialogs.h"
#include "JSONEditor.h"
#include "MusicBlock.h"
#include "PlayLists.h"
#include "QueueSystem.h"



/*void LoadTextureFromFile(const char* filename, SDL_Texture** texture_ptr, int& width, int& height, SDL_Renderer* renderer) {
    
    SDL_RWops* rw = SDL_RWFromConstMem(data, size);
    int channels;
    unsigned char* data = stbi_load(filename, &width, &height, &channels, 0);
    
    if (data == nullptr) {
        std::cout << "Failed to load image";
        throw std::exception("data");
    }

    SDL_PixelFormat format;
    if (channels == 3) {
        format = SDL_PIXELFORMAT_RGB24; 
    } else if (channels == 4) {
        format = SDL_PIXELFORMAT_RGBA32;
    } else {
        fprintf(stderr, "Unsupported image format (channels = %d)\n", channels);
        stbi_image_free(data);
        throw std::exception("SDL_PixelFormat");
    }

    SDL_Surface* surface = SDL_CreateSurfaceFrom(  
        width,          
        height, 
        format,       
        (void*)data, 
        width * channels
    );

    if (!surface) {
        std::cout << "Failed to create surface from image";
        stbi_image_free(data);
        throw std::exception("surface");
    }

    *texture_ptr = SDL_CreateTextureFromSurface(renderer, surface);
    if (!*texture_ptr) {
        std::cout << "Failed to create texture from surface";
        SDL_DestroySurface(surface);
        stbi_image_free(data);
        throw std::exception("SDL_CreateTextureFromSurface");
    }

    SDL_DestroySurface(surface);
    stbi_image_free(data);
}

void LoadTextureFromMemory(const unsigned char* data, size_t size, SDL_Texture** texture_ptr, int& width, int& height, SDL_Renderer* renderer) {
    
    SDL_IOStream* rw = SDL_IOFromConstMem(data, size);
    if (!rw) {
        std::cerr << "Не удалось создать RWops из памяти: " << SDL_GetError() << std::endl;
        throw std::exception("SDL_RWops");
    }

    SDL_Surface* surface = SDL_LoadBMP_IO(rw, 1);  
    if (!surface) {
        throw std::exception("IMG_Load_RW");
    }

    width = surface->w;
    height = surface->h;

    *texture_ptr = SDL_CreateTextureFromSurface(renderer, surface);
    if (!*texture_ptr) {
        std::cerr << "Ошибка создания текстуры из поверхности: " << SDL_GetError() << std::endl;
        SDL_DestroySurface(surface);
        throw std::exception("SDL_CreateTextureFromSurface");
    }

    SDL_DestroySurface(surface);  
}*/

struct COLORS {
  float r = 1, g = 1, b = 1;
};

int volumeLevel = 50;

std::string GetTimeAsTextForProgressBar(std::chrono::milliseconds time) {
  auto timeText = time.count();
  int64_t seconds = timeText / 60000;
  int64_t milliseconds = timeText % 60000 / 1000;
  std::string totalTime =
      std::to_string(seconds) + "m " + std::to_string(milliseconds) + "s";
  return totalTime;
}
std::filesystem::path OpenFileDialog(const char *title, size_t numberOfFilters,
                                     const char **filterPatterns,
                                     const char *filterDescription) {
#if defined(_WIN32)
  auto wtitle = WstringFromString(title);
  std::vector<std::wstring> wFilterStrings;
  wFilterStrings.reserve(numberOfFilters);
  std::vector<const wchar_t *> wFilters;
  wFilters.reserve(numberOfFilters);
  for (size_t i = 0; i < numberOfFilters; i++) {
    wFilterStrings.push_back(WstringFromString(filterPatterns[i]));
    wFilters.push_back(wFilterStrings[i].c_str());
  }
  std::wstring description = WstringFromString(filterDescription);
  auto *file = tinyfd_openFileDialogW(wtitle.c_str(), nullptr, numberOfFilters,
                                      wFilters.data(), description.c_str(), 0);
  if (!file) {
    return {};
  }
  return {file};
#else
  auto *file = tinyfd_openFileDialog(title, nullptr, numberOfFilters,
                                     filterPatterns, filterDescription, 0);
  if (!file) {
    return {};
  }
  return {file};
#endif
}

int main() {

  SetConsoleOutputToUnicode();
  COLORS cl;
  IWINDOW iwindow(762, 540, "");
  SDL_AudioSpec spec;
  spec.channels = 2;
  spec.format = MIX_DEFAULT_FORMAT;
  spec.freq = MIX_DEFAULT_FREQUENCY;
  if (Mix_OpenAudio(0, &spec) < 0) {
    std::cerr << "Ошибка инициализации SDL_mixer: " << Mix_GetError()
              << std::endl;
    SDL_Quit();
    return 1;
  }

  Queue queue;
  DeletionQueue LastFrame;

  bool ProcessChange = false;
  std::string nameplaylist;

  std::string TextButton = "Play";
  std::chrono::milliseconds CurrentTime{0};
  std::chrono::milliseconds TotalTime{0};

  std::function<void()> changing;

  MusicImporter Importer(std::filesystem::current_path() / "music",
                         std::filesystem::current_path() / "Library.json");

  auto functionThatRunsOnDragAndDrop = [&Importer](const std::string &path) {
    Importer.Import(path);
    JSONID jsonfile(Importer);
    jsonfile.Save(std::filesystem::current_path() / "Library.json");
  };
  iwindow.SetFunctionThatCallsOnDragAndDrop(functionThatRunsOnDragAndDrop);


  SDL_Texture* my_texture;
  int my_image_width, my_image_height;
  //LoadTextureFromFile("C:\\Users\\Dima\\Desktop\\Bruh.jpg", &my_texture, my_image_width, my_image_height, iwindow.Rendered());


  while (iwindow.IsRunning()) {
    iwindow.Events();
    iwindow.BeginDrawing();
    iwindow.StartFrame();
    ImGui::Begin("Gayteka");

    if (ImGui::BeginTabBar("TabBar")) {
      if (ImGui::BeginTabItem("Список треков")) {

        int my_value;
        if (ImGui::RadioButton("Date", my_value == 1))
          my_value = 1;
        ImGui::SameLine();
        if (ImGui::RadioButton("Name", my_value == 2))
          my_value = 2;
        auto temp_import = Importer.GetMusicList();
        if (my_value == 2)
          std::sort(temp_import.begin(), temp_import.end());

        for (auto &mus : temp_import) {
          ImGui::TextUnformatted(mus.c_str());
          if (ImGui::IsItemClicked()) {
            queue.AddToEnd(Importer.GetMusic(mus).GetUUID());
          }

          if (ImGui::IsItemHovered() &&
              ImGui::IsItemClicked(ImGuiMouseButton_Right)) {

            ImGui::OpenPopup(mus.c_str());
          }
          if (ImGui::BeginPopup(mus.c_str())) {

            if (ImGui::MenuItem("Change")) {
              ProcessChange = true;
              changing = [thisName = mus, mus, &ProcessChange,
                          &Importer]() mutable {
                ImGui::Begin("Change", &ProcessChange,
                             ImGuiWindowFlags_NoCollapse);

                if (ImGui::BeginPopupContextItem()) {
                  if (ImGui::MenuItem("Close Console"))
                    ProcessChange = false;
                  ImGui::EndPopup();
                }

                if (ImGui::InputText("Name", &thisName,
                                     ImGuiInputTextFlags_EnterReturnsTrue)) {
                  Importer.GetMusic(mus).ChangeName(thisName);
                }

                ImGui::End();
              };
            }

            if (ImGui::MenuItem("Delete")) {
              LastFrame.InputFunction(
                  [&Importer, mus]() { Importer.DeleteMusic(mus); });
            }
            ImGui::EndPopup();
          }

        } // Библиотека
        ImGui::EndTabItem();
      }

      if (ImGui::BeginTabItem("Плейлисты")) {
        auto workDirectory =
            std::filesystem::current_path() / "PlaylistCollection";
        if (!std::filesystem::exists(workDirectory)) {
          std::filesystem::create_directory(workDirectory);
        }

        for (auto &directoryEntry :
             std::filesystem::directory_iterator(workDirectory)) {
          std::ifstream ifs(directoryEntry.path(), std::ios::in);
          if (!ifs) {
            throw std::runtime_error("Cyka, cannot open file!");
          }
          nlohmann::json j;
          ifs >> j;
          ifs.close();

          ImGui::SetNextItemAllowOverlap();
          std::string filenameWithoutExtension =
              directoryEntry.path().stem().string();
          ImGui::Selectable(filenameWithoutExtension.c_str());

          if (ImGui::IsItemClicked()) {
            queue.CleanALL();
            for (const auto &obj : j) {
              queue.AddToEnd(obj.at("track_id").get<uint64_t>());
            }
          }
          ImGui::SameLine();
          ImGui::PushStyleColor(ImGuiCol_Button,
                                ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
          ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                                ImVec4(0.7f, 0.7f, 0.7f, 1.0f));
          ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                                ImVec4(0.7f, 0.7f, 0.7f, 1.0f));
          if (ImGui::SmallButton("x")) {
            LastFrame.InputFunction([path = directoryEntry.path()]() {
              std::filesystem::remove(path);
            });
          }
          ImGui::PopStyleColor(3);
        }

        if (ImGui::InputText("Name of playlist", &nameplaylist))
          ;
        if (ImGui::Button("+")) {
          Playlists list(queue.getMainqueue());
          list.Save(std::filesystem::current_path() / "PlaylistCollection" /
                    (nameplaylist + ".json"));
        }
        ImGui::SameLine();
        ImGui::Text("Сохранить текущую очередь как плейлист");
        ImGui::EndTabItem();
      }
    }
    ImGui::EndTabBar();

    ImGui::End();

    ImGui::SetNextWindowSize({480, 720});
    ImGui::Begin("Bruh", nullptr, ImGuiWindowFlags_NoTitleBar);
    auto currentMusic = queue.GetCurrentMusic();
    std::string currentMusName =
        currentMusic == 0 ? "Choose music"
                          : Importer.GetMusic(currentMusic).GetName();

    // ImGui::SetCursorPos({ImGui::GetWindowSize().x/2, 30});
    ImGui::TextUnformatted(currentMusName.c_str());

    if (currentMusName == "Choose music") {
      ImGui::ProgressBar(0, ImVec2(200.0f, 20.0f));
      ImGui::Text("0m/ 0s");
    } else {
      TotalTime = Importer.GetMusic(currentMusic).GetTime();
      CurrentTime = std::chrono::seconds(static_cast<int64_t>(
          Mix_GetMusicPosition(Importer.GetMusic(currentMusic).GetMus())));

      ImGui::ProgressBar(static_cast<float>(CurrentTime.count()) /
                             static_cast<float>(TotalTime.count()),
                         ImVec2(200.0f, 20.0f),
                         ""); // хз почему nullptr не робит

      if ((ImGui::IsItemHovered()) and (ImGui::IsMouseClicked(0))) {

        auto BarPosX = ImGui::GetCursorScreenPos().x + 200;

        auto pos = ImGui::GetMousePos().x;

        auto timer = (BarPosX - pos) / 200;

        Mix_SetMusicPosition(TotalTime.count() * (1 - timer) / 1000);
      }
      std::string totalTime = GetTimeAsTextForProgressBar(TotalTime);

      std::string currentTime = GetTimeAsTextForProgressBar(CurrentTime);

      ImGui::Text("%s/%s", currentTime.c_str(), totalTime.c_str());

      if (TotalTime == CurrentTime) {
        if (queue.isLoop())
        {
          Mix_SetMusicPosition(0);
        }
        else
         {
          
            Mix_RewindMusic();
            Mix_PauseMusic();
            queue.Next_Song();
            currentMusic = queue.GetCurrentMusic();
            if (currentMusic != 0) {
              Mix_PlayMusic(Importer.GetMusic(currentMusic).GetMus(), 1);
              TotalTime = Importer.GetMusic(currentMusic).GetTime();
            }
         }
      }
    }

    ImGui::SetCursorPos({420, 20});

    if (ImGui::Button("Down", {40, 40})) {
      const char *filterPatterns[2] = {"*.mp3", "*.WAV"};
      std::filesystem::path result =
          OpenFileDialog("Напишите", 2, filterPatterns, ".mp3,.WAV");
      if (!result.empty()) {
        Importer.Import(result);
        JSONID jsonfile(Importer);
        jsonfile.Save(std::filesystem::current_path() / "Library.json");
      }
    }

    if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
      ImGui::SetTooltip("Загрузить звуковой файл");
    }

    ImGui::SetCursorPos({100, 70});
    if (ImGui::Button("CleanAll")) {
      queue.CleanALL();
    }

    static bool isPlaying = false;
    if (Mix_PlayingMusic() && !Mix_PausedMusic() != isPlaying) {
      isPlaying = Mix_PlayingMusic() && !Mix_PausedMusic();
      if (isPlaying)
        TextButton = "Stop";
      else {
        TextButton = "Play";
      }
    }
    if (ImGui::Button(TextButton.c_str(), {100, 50}) and (currentMusic != 0)) {
      Play(Importer.GetMusic(currentMusic).GetMus());
    }

    ImGui::SameLine();
    if (ImGui::Button("Random", {100, 50}))
      queue.Randomize();

    ImGui::SameLine();
    if (ImGui::Button("<", {10, 40})) {
      Mix_RewindMusic();
      Mix_PauseMusic();
      queue.Prev_Song();
      if (currentMusic != 0) {
        currentMusic = queue.GetCurrentMusic();
        Mix_PlayMusic(Importer.GetMusic(currentMusic).GetMus(), 1);
        TotalTime = Importer.GetMusic(currentMusic).GetTime();
      }
    }
    ImGui::SameLine();
    if (ImGui::Button(">", {10, 40})) {
      Mix_RewindMusic();
      Mix_PauseMusic();
      queue.Next_Song();
    }

    ImGui::SameLine();
    if (ImGui::RadioButton("Repeater",queue.isLoop()))
      {
        queue.ChangeLoop();
      }
    ImGui::SameLine();
    if (ImGui::RadioButton("Same Repeater But Cooler",queue.isQueueLoop()))
      {
        queue.ChangeQueueLoop();
      }
    if (ImGui::SliderInt("##volume", &volumeLevel, 0, 100))
      Mix_VolumeMusic(volumeLevel);

    for (auto [musicInternalId, musicId] : queue.getMainqueue()) {
      auto &music = Importer.GetMusic(musicId);
      auto &musicName = music.GetName();
      std::string internalIdString = std::to_string(musicInternalId);

      ImGui::TextUnformatted(musicName.c_str());

      if (ImGui::IsItemClicked()) {
        TotalTime = music.GetTime();
        queue.New_Crt_Mus(musicInternalId);
        // was isPlaying
        Mix_PauseMusic();
        Mix_RewindMusic();

        Mix_PlayMusic(music.GetMus(), 1);
      }

      ImGui::SameLine();
      ImGui::PushID(internalIdString.c_str());

      if (ImGui::Button("Delete")) {
        LastFrame.InputFunction(
            [&queue, musicInternalId]() { queue.Delete(musicInternalId); });

        if (currentMusic == musicId) {
          Mix_RewindMusic();
          Mix_PauseMusic();
        }
      }

      ImGui::SameLine();
      if (!queue.IsFirst(musicInternalId) and ImGui::Button("Up")) {
        queue.SwapUp(musicInternalId);
      }

      ImGui::SameLine();
      if (!queue.IsLast(musicInternalId) and ImGui::Button("Down")) {
        queue.SwapDown(musicInternalId);
      }


      ImGui::PopID();
    }

    // SDL_Texture* my_texture;
    // int my_image_width, my_image_height;
    // LoadTextureFromFile("C:\\Users\\Dima\\Desktop\\7de3058f007abcc70e7ebce877d19c68.jpg", &my_texture, my_image_width, my_image_height, iwindow.Rendered());
    // float aspectRatio = static_cast<float>(my_image_width) / static_cast<float>(my_image_height);
    // auto imageSize = ImGui::GetContentRegionAvail();
    // imageSize.y = imageSize.x / aspectRatio;

    // SDL_Texture* textureToShow;
    ///...
    
    //ImGui::Image((void*) textureToShow, imageSize);
    //ImGui::ImageButton("SD", (void*) my_texture, imageSize);
    
    if (ProcessChange) {
      changing();
    }
    LastFrame.OutputFunction();

    ImGui::End();

    // Start ImGui frame

    iwindow.EndDrawing(cl.r * 255, cl.g * 255, cl.b * 255,
                       255); // 3 int: 0 - 255
  }
  // Cleanup
}
