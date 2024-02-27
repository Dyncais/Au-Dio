//
// Created by Dima on 17.11.2023.
//

#include "filesystem"
#include "iostream"
#include "SDL3_mixer/SDL_mixer.h"
#include "chrono"
#include "UUID.h"

#pragma once

class MusicBlock
{
    std::string name;

    std::filesystem::path path;

    std::chrono::milliseconds duration {0};

    Mix_Music* object = nullptr;

    const MusicPlayer::UUID uuid;
public:
    const MusicPlayer::UUID &GetUUID() const
    {
        return uuid;
    }

public:

    MusicBlock() = default;

    Mix_Music* GetMus()
    {
        return object;
    }

    std::chrono::milliseconds GetTime()
    {
        return duration;
    }

    const std::string& GetName()
    {
        return name;
    }

    const  std::filesystem::path& GetPath()
    {
        //лол блять, тут пути то и нет по факту
        return path;
    }

    MusicBlock(const std::filesystem::path& path)
    {
        this->path = path;
        object = Mix_LoadMUS(path.string().c_str());
        name = Mix_GetMusicTitle(object);

        if(name.empty())
            name = std::filesystem::path(path).filename().replace_extension("").string();

        double time = Mix_MusicDuration(object);

        duration = std::chrono::seconds (static_cast<int64_t>(time));

        if (object == NULL) {
            std::cerr << "Ошибка при загрузке MP3: " << Mix_GetError() << std::endl;
            return;
        }

    }

    MusicBlock(MusicBlock&& original)
    {
        name = std::move(original.name);

        path = std::move(original.path);

        object = original.object;

        duration = original.duration;

        original.object = nullptr;
    }


    MusicBlock& operator=(MusicBlock&& original)
    {
        name = std::move(original.name);

        path = std::move(original.path);

        if (object!= nullptr)
            Mix_FreeMusic(object);

        object = original.object;

        original.object = nullptr;

        duration = original.duration;

        return *this;
    }

    void ChangeName(std::string newName)
    {
        name = newName;
    }

    ~MusicBlock()
    {
        if(object == nullptr)
            return;

        Mix_FreeMusic(object);
    }
};