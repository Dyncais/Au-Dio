//
// Created by Dima on 17.11.2023.
//

#include "filesystem"
#include "iostream"
#include <fstream>
#include "SDL3_mixer/SDL_mixer.h"
#include "chrono"
#include "UUID.h"

#include "stb_image.h"
#include <taglib/attachedpictureframe.h>
#include "taglib/tiostream.h"
#include <taglib/fileref.h>
#include <taglib/mpegfile.h>
#include <taglib/id3v2tag.h>
#include "taglib/mpegfile.h"
#include <cstdio>

#pragma once



class MusicBlock
{
    std::string name;

    std::filesystem::path path;

    std::chrono::milliseconds duration {0};

    const unsigned char* imagedata;

    Mix_Music* object = nullptr;

    MusicPlayer::UUID uuid;
public:
    const MusicPlayer::UUID &GetUUID() const
    {
        return uuid;
    }

    void SetIDFromJSON(MusicPlayer::UUID newuuid)
    {
        uuid = newuuid;
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
        //лол, тут пути то и нет по факту
        return path;
    }

    const unsigned char*& GetImage()
    {
        return imagedata;
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
        TagLib::MPEG::File file(path.string().c_str());
        if (file.isValid()) {
            TagLib::ID3v2::Tag* id3v2tag = file.ID3v2Tag();
            if (id3v2tag) {
                TagLib::ID3v2::FrameList frames = id3v2tag->frameList("APIC");
                if (!frames.isEmpty()) {
                    auto* picFrame = dynamic_cast<TagLib::ID3v2::AttachedPictureFrame*>(frames.front());
                    if (picFrame) {
                        // std::string coverPath = "C:\\Users\\Dima\\Desktop\\cover.jpg";
                        // std::ofstream imageFile(coverPath, std::ios::binary);
                        // imageFile.write((const char*)picFrame->picture().data(), picFrame->picture().size());
                        // imageFile.close();
                        imagedata = reinterpret_cast<const unsigned char*>(picFrame->picture().data());

                    } else {
                        std::cerr << "Не удалось преобразовать фрейм в AttachedPictureFrame." << std::endl;
                    }
                } else {
                    std::cout << "Обложка не найдена." << std::endl;
                }
            }
        } else {
            std::cerr << "Не удалось прочитать метаданные." << std::endl;
        }

        std::cout << std::format("MusicBlock created. UUID: {0}; Name: {1}; Path: {2}; Duration {3} ms", static_cast<uint64_t>(uuid), name, path.string(), duration) << '\n';
    }

    MusicBlock(MusicBlock&& original)
    : uuid(original.uuid)
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

        uuid = original.uuid;

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
        std::cout << std::format("MusicBlock Deleted. UUID: {0}; Name: {1}; Path: {2}; Duration {3} ms", static_cast<uint64_t>(uuid), name, path.string(), duration) << '\n';
    }
};