#include <iostream>
#include "IWINDOW.h"
#include "import.h"
#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"
#include <filesystem>

#include <windows.h>
#include <shellapi.h>
#include <SDL.h>

#include "DeletionQueue.h"
#include "MusicBlock.h"
#include "QueueSystem.h"
#include "JSONEditor.h"
#include "PlayLists.h"
#include "ForWindows/tinyfiledialogs.h"
#include "chrono"
#include "ForWindows/Windows.h"
#include "ForWindows/json.hpp"

struct COLORS
{
    float r = 1, g = 1, b = 1;
};

int volumeLevel = 50;

NOTIFYICONDATA nid;

std::string GetTimeAsTextForProgressBar(std::chrono::milliseconds time)
{
    auto timeText = time.count();
    int64_t seconds = timeText / 60000;
    int64_t milliseconds = timeText % 60000 / 1000;
    std::string totalTime = std::to_string(seconds) + "m " + std::to_string(milliseconds) + "s";
    return totalTime;
}

int main()
{

    SetConsoleOutputToUnicode();
    COLORS cl;
    IWINDOW iwindow(762,540,"");
    SDL_AudioSpec spec;
    spec.channels = 2;
    spec.format = MIX_DEFAULT_FORMAT;
    spec.freq = MIX_DEFAULT_FREQUENCY;
    if (Mix_OpenAudio(0, &spec) < 0) {
        std::cerr << "Ошибка инициализации SDL_mixer: " << Mix_GetError() << std::endl;
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

    MusicImporter Importer(std::filesystem::current_path() / "music",std::filesystem::current_path() / "Library.json");


    auto functionThatRunsOnDragAndDrop = [&Importer](const std::string& path){
        Importer.Import(path);
        JSONID jsonfile(Importer);
        jsonfile.Save(std::filesystem::current_path() / "Library.json");
    };
    iwindow.SetFunctionThatCallsOnDragAndDrop(functionThatRunsOnDragAndDrop);


    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = (HWND)(SDL_GetProperty(SDL_GetWindowProperties(iwindow.window),"SDL.window.win32.hwnd",nullptr));
    nid.uID = 15;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = 15666;
    std::string iconPath = (std::filesystem::current_path() / "Assets/Icon.ico").string();
    nid.hIcon = (HICON)LoadImage(nullptr, iconPath.c_str(), IMAGE_ICON, 0, 0, LR_LOADFROMFILE);

    strcpy_s(nid.szTip, "Player");
    Shell_NotifyIcon(NIM_ADD, &nid);


    //ShowWindow(nid.hWnd, SW_HIDE);
    //ShowWindow(nid.hWnd, SW_RESTORE);
    //HMENU hMenu = LoadMenu(nullptr, iconPath.c_str());

    while(iwindow.IsRunning())
    {
        iwindow.Events();
        iwindow.StartFrame();
        ImGui::Begin("Gayteka");

        if (ImGui::BeginTabBar("TabBar"))
        {
            if (ImGui::BeginTabItem("Список треков"))
            {

                int my_value;
                if (ImGui::RadioButton("Date", my_value == 1))
                    my_value = 1;
                ImGui::SameLine();
                if (ImGui::RadioButton("Name", my_value == 2))
                    my_value = 2;
                auto temp_import = Importer.GetMusicList();
                if (my_value == 2)
                    std::sort(temp_import.begin(), temp_import.end());

                for (auto &mus: temp_import)
                {
                    ImGui::Text(mus.c_str());
                    if (ImGui::IsItemClicked())
                    {
                        queue.AddToEnd(Importer.GetMusic(mus).GetUUID());
                    }

                    if (ImGui::IsItemHovered() && ImGui::IsItemClicked(ImGuiMouseButton_Right))
                    {

                        ImGui::OpenPopup(mus.c_str());

                    }
                    if (ImGui::BeginPopup(mus.c_str()))
                    {

                        if (ImGui::MenuItem("Change"))
                        {
                            ProcessChange = true;
                            changing = [thisName = mus, mus, &ProcessChange, &Importer]() mutable
                            {
                                ImGui::Begin("Change", &ProcessChange, ImGuiWindowFlags_NoCollapse);


                                if (ImGui::BeginPopupContextItem())
                                {
                                    if (ImGui::MenuItem("Close Console"))
                                        ProcessChange = false;
                                    ImGui::EndPopup();
                                }

                                if (ImGui::InputText("Name", &thisName, ImGuiInputTextFlags_EnterReturnsTrue))
                                {
                                    Importer.GetMusic(mus).ChangeName(thisName);
                                }

                                ImGui::End();
                            };
                        }

                        if (ImGui::MenuItem("Delete"))
                        {
                            LastFrame.InputFunction([&Importer, mus]()
                                                    {
                                                        Importer.DeleteMusic(mus);
                                                    });

                        }
                        ImGui::EndPopup();
                    }

                } //Библиотека
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Плейлисты"))
            {
                auto workDirectory = std::filesystem::current_path() / "PlaylistCollection";
                if(!std::filesystem::exists(workDirectory))
                {
                    std::filesystem::create_directory(workDirectory);
                }

                for(auto& directoryEntry : std::filesystem::directory_iterator(workDirectory))
                {
                    std::ifstream ifs(directoryEntry.path(), std::ios::in);
                    if (!ifs)
                    {
                        throw std::runtime_error("Cyka, cannot open file!");
                    }
                    nlohmann::json j;
                    ifs >> j;
                    ifs.close();

                    std::string filenameWithoutExtension = directoryEntry.path().stem().string();
                    ImGui::Selectable(filenameWithoutExtension.c_str());

                    if (ImGui::IsItemClicked())
                    {
                        queue.CleanALL();
                        for (const auto &obj: j)
                        {
                            queue.AddToEnd(obj.at("track_id").get<uint64_t>());
                        }

                    }
                }

                if (ImGui::InputText("Name of playlist", &nameplaylist))
                {

                }

                if(ImGui::Button("+"))
                {
                    Playlists list(queue.getMainqueue());
                    list.Save(std::filesystem::current_path() / "PlaylistCollection" / (nameplaylist + ".json"));
                }
                ImGui::SameLine();
                ImGui::Text("Сохранить текущую очередь как плейлист");
                ImGui::EndTabItem();
            }
        }
        ImGui::EndTabBar();

        ImGui::End();

        ImGui::SetNextWindowSize({480 ,720});
        ImGui::Begin("Bruh", nullptr, ImGuiWindowFlags_NoTitleBar);
        auto currentMusic = queue.GetCurrentMusic();
        std::string currentMusName = currentMusic == 0 ? "Choose music" :  Importer.GetMusic(currentMusic).GetName();

        //ImGui::SetCursorPos({ImGui::GetWindowSize().x/2, 30});
        ImGui::Text(currentMusName.c_str());

        if(currentMusName == "Choose music")
        {
            ImGui::ProgressBar(0, ImVec2(200.0f, 20.0f));
            ImGui::Text("0m/ 0s");
        }
        else
        {
            TotalTime = Importer.GetMusic(currentMusic).GetTime();
            CurrentTime = std::chrono::seconds (static_cast<int64_t>(Mix_GetMusicPosition(Importer.GetMusic(currentMusic).GetMus())));

            ImGui::ProgressBar(static_cast<float>(CurrentTime.count()) / static_cast<float>(TotalTime.count()), ImVec2(200.0f, 20.0f),""); //хз почему nullptr не робит

            std::string totalTime = GetTimeAsTextForProgressBar(TotalTime);

            std::string currentTime = GetTimeAsTextForProgressBar(CurrentTime);

            ImGui::Text("%s/%s", currentTime.c_str(),totalTime.c_str());

            if(TotalTime == CurrentTime)
            {
                Mix_RewindMusic();
                Mix_PauseMusic();
                queue.Next_Song();
                currentMusic = queue.GetCurrentMusic();
                if (currentMusic != 0)
                {
                    Mix_PlayMusic(Importer.GetMusic(currentMusic).GetMus(), 1);
                    TotalTime = Importer.GetMusic(currentMusic).GetTime();
                }
            }

        }

        ImGui::SetCursorPos({420,20});

        if(ImGui::Button("Down",{40, 40}))
        {
            const char * filterPatterns[2] = { "*.mp3", "*.WAV" };
            const char * result = tinyfd_openFileDialog("Напишите", nullptr, 2, filterPatterns,".mp3,.WAV",0);
            if (result) {
                Importer.Import(result);
            }
        }

        if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
        {
            ImGui::SetTooltip("Загрузить звуковой файл");
        }



        ImGui::SetCursorPos({100,70});
        static bool isPlaying = false;
        if(Mix_PlayingMusic() && !Mix_PausedMusic() != isPlaying)
        {
            isPlaying = Mix_PlayingMusic() && !Mix_PausedMusic();
            if(isPlaying)
                TextButton = "Stop";
            else
            {
                TextButton = "Play";
            }
        }
        if(ImGui::Button(TextButton.c_str(),{100, 50}) and (currentMusic != 0))
        {
            Play(Importer.GetMusic(currentMusic).GetMus());
        }

        ImGui::SameLine();
        if(ImGui::Button("Random",{100, 50}))
            queue.Randomize();

        ImGui::SameLine();
        if(ImGui::Button("<",{10, 40}))
        {
            Mix_RewindMusic();
            Mix_PauseMusic();
            queue.Prev_Song();
            if (currentMusic != 0)
            {
                currentMusic = queue.GetCurrentMusic();
                Mix_PlayMusic(Importer.GetMusic(currentMusic).GetMus(), 1);
                TotalTime = Importer.GetMusic(currentMusic).GetTime();
            }
        }

        if (ImGui::SliderInt("##volume", &volumeLevel, 0, 100))
            Mix_VolumeMusic(volumeLevel);


        for(auto [musicInternalId, musicId] :  queue.getMainqueue())
        {
            auto& music = Importer.GetMusic(musicId);
            auto& musicName = music.GetName();
            std::string internalIdString = std::to_string(musicInternalId);

            ImGui::Text(musicName.c_str());

            if (ImGui::IsItemClicked())
            {
                TotalTime = music.GetTime();
                queue.New_Crt_Mus(musicInternalId);
                //was isPlaying
                Mix_PauseMusic();
                Mix_RewindMusic();

                Mix_PlayMusic(music.GetMus(), 1);
            }

            ImGui::SameLine();
            ImGui::PushID(internalIdString.c_str());

            if(ImGui::Button("Delete"))
            {
                LastFrame.InputFunction([&queue,musicInternalId](){
                    queue.Delete(musicInternalId);
                });

                if (currentMusic == musicId)
                {
                    Mix_RewindMusic();
                    Mix_PauseMusic();
                }
            }


            ImGui::SameLine();
            if(!queue.IsFirst(musicInternalId) and ImGui::Button("Up"))
            {
                queue.SwapUp( musicInternalId);
            }


            ImGui::SameLine();
            if(!queue.IsLast(musicInternalId) and ImGui::Button("Down"))
            {
                queue.SwapDown( musicInternalId);
            }

            ImGui::PopID();

        }

        if (ProcessChange)
        {
            changing();
        }
        LastFrame.OutputFunction();

        ImGui::End();

        //Start ImGui frame

        iwindow.Drawing(cl.r * 255,cl.g * 255,cl.b * 255,255); // 3 int: 0 - 255
    }
    Shell_NotifyIcon(NIM_DELETE, &nid);


    //Cleanup

}
