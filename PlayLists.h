//
// Created by Dima on 10.03.2024.
//

#include "ForWindows/json.hpp"
#include "MusicBlock.h"
#include "import.h"
#include "fstream"

#pragma once


class Playlists
{
private:
    nlohmann::json v = nlohmann::json::array();

public:
    Playlists(std::vector<std::pair<MusicPlayer::UUID, MusicPlayer::UUID>>  queue)
    {
            for (auto& i : queue)
            {
                nlohmann::json obj = {
                        {"playlist_id", static_cast<uint64_t>(i.first)}, {"track_id", static_cast<uint64_t>(i.second)}
                };
                v.push_back(obj);
            }
    }

    void Save(const std::filesystem::path& path)
    {
        std::ofstream ofs(path,std::ios::out);
        if(!ofs)
            throw std::runtime_error("Blyat(");
        ofs << v.dump(4);
        ofs.close();
    }
};