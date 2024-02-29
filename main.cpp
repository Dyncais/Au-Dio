#include <iostream>
#include "IWINDOW.h"
#include "import.h"
#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"
#include <filesystem>

#include <windows.h>
#include <shellapi.h>
#include <SDL.h>

#include "MusicBlock.h"
#include "QueueSystem.h"
#include "ForWindows/tinyfiledialogs.h"
#include "chrono"
#include "ForWindows/Windows.h"

struct COLORS
{
    float r = 1, g = 1, b = 1;
};

int volumeLevel = 50;

NOTIFYICONDATA nid;

std::function<void()> deletionfromsys;

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
    bool isDragAndDrop = false;
    std::string dragAndDropPath;
    auto functionThatRunsOnDragAndDrop = [&isDragAndDrop, &dragAndDropPath](const std::string& path){
        isDragAndDrop = true;
        dragAndDropPath = path;
    };
    iwindow.SetFunctionThatCallsOnDragAndDrop(functionThatRunsOnDragAndDrop);

    std::string TextButton = "Play";
    std::chrono::milliseconds CurrentTime{0};
    std::chrono::milliseconds TotalTime{0};

    bool ProcessChange = false;
    bool DeletionFileSys = false;
    std::function<void()> changing;

    MusicImporter Importer(std::filesystem::current_path() / "music");


    Queue queue;




    while(iwindow.IsRunning())
    {


        nid.cbSize = sizeof(NOTIFYICONDATA);
        nid.hWnd = (HWND)(SDL_GetProperty(SDL_GetWindowProperties(iwindow.window),"SDL.window.win32.hwnd",NULL));
        nid.uID = 15;
        nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
        nid.uCallbackMessage = 15666;
        nid.hIcon = (HICON)LoadImage(NULL, "C:\\Users\\Dima\\CLionProjects\\AudioPlay\\Icon.ico", IMAGE_ICON, 0, 0, LR_LOADFROMFILE);

        strcpy_s(nid.szTip, "Player");
        Shell_NotifyIcon(NIM_ADD, &nid);

        iwindow.Events();
        iwindow.StartFrame();

        ImGui::Begin("Gayteka");

        int my_value;
        if (ImGui::RadioButton("Date", my_value==1))
        { my_value = 1;
             }
        ImGui::SameLine();
        if (ImGui::RadioButton("Name", my_value==2))
        {
            my_value = 2;
        }
        auto temp_import = Importer.GetMusicList();
        if(my_value == 2)
            std::sort(temp_import.begin(), temp_import.end());

        for(auto& mus : temp_import)
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
                    changing = [thisName = mus, mus, &ProcessChange,&Importer]() mutable
                    {
                        ImGui::Begin("Change",&ProcessChange, ImGuiWindowFlags_NoCollapse);


                        if (ImGui::BeginPopupContextItem())
                        {
                            if (ImGui::MenuItem("Close Console"))
                                ProcessChange = false;
                            ImGui::EndPopup();
                        }

                        if (ImGui::InputText("Name",&thisName, ImGuiInputTextFlags_EnterReturnsTrue))
                        {
                            Importer.GetMusic(mus).ChangeName(thisName);
                        };

                        ImGui::End();
                    };

                }

                if (ImGui::MenuItem("Delete"))
                {
                    deletionfromsys = [&Importer,mus]() {
                        Importer.DeleteMusic(mus);
                    };
                    DeletionFileSys = true;
                    //Importer.DeleteMusic(mus);
                }
                ImGui::EndPopup();
            }

        } //Библиотека
        ImGui::End();

        ImGui::SetNextWindowSize({480 ,720});
        ImGui::Begin("Bruh");
        auto currentMusic = queue.GetCurrentMusic();
        std::string currentMusName = currentMusic == 0 ? "Choose music" :  Importer.GetMusic(currentMusic).GetName();

        //ImGui::SetCursorPos({ImGui::GetWindowSize().x/2, 30});
        ImGui::Text(currentMusName.c_str());
        if(currentMusName != "Choose music")
        {
            CurrentTime = std::chrono::seconds (static_cast<int64_t>(Mix_GetMusicPosition(Importer.GetMusic(currentMusic).GetMus())));
        }
        auto timeText = TotalTime.count();
        int seconds = timeText / 60000;
        int milliseconds = timeText % 60000 / 1000;

        std::string totalTime = std::to_string(seconds) + "m " + std::to_string(milliseconds) + "s";
        timeText = CurrentTime.count();
        seconds = timeText / 60000;
        milliseconds = timeText % 60000 / 1000;
        std::string currentTime = std::to_string(seconds) + "m " + std::to_string(milliseconds) + "s";

        if(TotalTime.count() == 0)
            ImGui::ProgressBar(0, ImVec2(200.0f, 20.0f));
        else if(TotalTime == CurrentTime)
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
        else
        {
            //ImGui::SetCursorPosX(ImGui::GetWindowSize().x/2);
            ImGui::ProgressBar(static_cast<float>(CurrentTime.count()) / static_cast<float>(TotalTime.count()), ImVec2(200.0f, 20.0f),""); //хз почему nullptr не робит

        }

        ImGui::Text("%s/%s", currentTime.c_str(),totalTime.c_str());



        ImGui::SetCursorPos({420,20});

        if(ImGui::Button("Down",{40, 40}))
        {
            const char * filterPatterns[2] = { "*.mp3", "*.WAV" };
            const char * result = tinyfd_openFileDialog("Напишите", NULL, 2, filterPatterns,".mp3,.WAV",0);
            if (result) {
                Importer.Import(result);
                //NameMus = std::filesystem::path(result).filename().replace_extension("").string();
                //CurrentTime = current_mus.GetTime();

            }
        };

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
                TextButton = "Play";
        }
        if(ImGui::Button(TextButton.c_str(),{100, 50}) and (currentMusic != 0))
        {
            Play(Importer.GetMusic(currentMusic).GetMus());
        };

        ImGui::SameLine();
        if(ImGui::Button("Random",{100, 50}))
            queue.Randomize();

        ImGui::SameLine();
        if(ImGui::Button("<",{10, 40}))
        {
            Mix_RewindMusic();
            Mix_PauseMusic();
            queue.Prev_Song();
            currentMusic = queue.GetCurrentMusic();
            if (currentMusic != 0)
            {
              Mix_PlayMusic(Importer.GetMusic(currentMusic).GetMus(), 1);
               TotalTime = Importer.GetMusic(currentMusic).GetTime();
            }
        }

        if (ImGui::SliderInt("##volume", &volumeLevel, 0, 100))
            Mix_VolumeMusic(volumeLevel);


        std::function<void()> deletion;
        bool ProcessDelete = false;
        size_t count = 0;
        bool canShow = false;
        for(auto [musicInternalId, musicId] :  queue.getMainqueue())
        {
            auto& music = Importer.GetMusic(musicId);
            auto& musicName = music.GetName();
            std::string internalIdString = std::to_string(musicInternalId);
            /*if(!canShow && current != "##")
            {
                if(music == current)
                {
                    canShow = true;
                }
                count++;
                continue;
            }*/
            ImGui::Text(musicName.c_str());

            if (ImGui::IsItemClicked())
            {
                TotalTime = music.GetTime();
                queue.New_Crt_Mus(musicInternalId);
                if(isPlaying)
                {
                    Mix_PauseMusic();
                    Mix_RewindMusic();
                }
                Mix_PlayMusic(music.GetMus(), 1);
            }

            ImGui::SameLine();
            ImGui::PushID(internalIdString.c_str());

            if(ImGui::Button("Delete"))
            {
                 deletion =[&queue,musicInternalId](){
                    queue.Delete(musicInternalId);
                };
                ProcessDelete = true;
            }

            if (count != 0)
            {
                ImGui::SameLine();
                if(ImGui::Button("Up"))
                {
                    queue.Swap( count, count - 1);
                }
            }

            if(count !=  queue.getMainqueue().size() - 1)
            {
                ImGui::SameLine();
                if(ImGui::Button("Down"))
                {
                    queue.Swap( count, count + 1);
                }
            }

            count++;

            ImGui::PopID();



        }

        if(ProcessDelete)
        {
            deletion();
            ProcessDelete = false;
        }
        if (ProcessChange)
        {
            changing();
        }

        if (DeletionFileSys)
        {
            deletionfromsys();
            DeletionFileSys = false;
        }

        if(isDragAndDrop)
        {
            Importer.Import(dragAndDropPath);

            isDragAndDrop = false;

        }

        ImGui::End();

        //Start ImGui frame

        iwindow.Drawing(cl.r * 255,cl.g * 255,cl.b * 255,255); // 3 int: 0 - 255
    }
    //Cleanup

}
