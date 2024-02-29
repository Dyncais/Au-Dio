//
// Created by Dima on 11.11.2023.
//
#include "vector"
#include "MusicBlock.h"
#include "iostream"
#include "SDL3_mixer/SDL_mixer.h"
#pragma once

class MusicImporter
{

public:

    MusicImporter(const std::filesystem::path& workingDirectory)
    {
        workDirectory = workingDirectory;

        if(!std::filesystem::exists(workDirectory))
        {
            std::filesystem::create_directory(workDirectory);
        }

        for(auto& directoryEntry : std::filesystem::directory_iterator(workDirectory))
        {
            Collector.emplace_back(directoryEntry.path());
        }
    }

    void Import(const std::string& path)
    {
        SaveMusicIfNotSaved(path);

    }

    MusicBlock& GetMusic(const std::string& Name)
    {
        for(int i=0;i<Collector.size();i++)
        {
            if (Name == Collector[i].GetName())
            {
                return Collector[i];
            };
        }
        throw std::runtime_error("Could not find music in Importer");
    }
    MusicBlock& GetMusic(MusicPlayer::UUID id)
    {
        for(int i=0;i<Collector.size();i++)
        {
            if (id == Collector[i].GetUUID())
            {
                return Collector[i];
            };
        }

        throw std::runtime_error("Could not find music in Importer");
    }

    std::vector<std::string> GetMusicList()
    {
        std::vector<std::string> tempList;
        for(int i=0;i<Collector.size();i++)
        {
            tempList.push_back(Collector[i].GetName());

        }
        return tempList;
    }

    std::vector<MusicPlayer::UUID> GetMusicUUIDList()
    {
        std::vector<MusicPlayer::UUID> tempList;
        for(const auto & i : Collector)
        {
            tempList.push_back(i.GetUUID());

        }
        return tempList;
    }

    std::vector<MusicBlock>& GetCollection()
    {
        return Collector;
    }

    void DeleteMusic(const std::string& Name)
    {
        for(int i=0;i<Collector.size();i++)
        {
            if (Name == Collector[i].GetName())
            {
                auto temp_path = Collector[i].GetPath();
                Collector.erase(Collector.begin() +  i);
                std::filesystem::remove_all(temp_path);
                return;
            };
        }
    }
private:

    std::filesystem::path workDirectory;
    std::vector<MusicBlock> Collector;

    void SaveMusicIfNotSaved(const std::filesystem::path& path)
    {
        auto name = std::filesystem::path(path).filename();
        auto internalPath = workDirectory / name;


        if(std::filesystem::exists(internalPath))
        {
            return;
        }
        std::filesystem::copy_file(path,internalPath);

        Collector.emplace_back(internalPath);
    }

};


void Play(Mix_Music* music)
{
    if(Mix_PausedMusic())
    {
        Mix_ResumeMusic();
        return;
    }
    if(Mix_PlayingMusic())
    {
        Mix_PauseMusic();
        return;
    }

    Mix_PlayMusic(music, 1); // -1 означает бесконечное воспроизведение

    // Ожидание, пока музыка играет
    //SDL_Delay(5000); // 5 секунд

    // Очистка
    //Mix_FreeMusic(music);
}

