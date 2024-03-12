//
// Created by Dima on 10.03.2024.
//

#pragma once
#include "ForWindows/json.hpp"
#include "import.h"
#include "fstream"

class JSONID
{
private:

    nlohmann::json v = nlohmann::json::array();
public:

    JSONID(MusicImporter& Importer)
    {
        for (auto& i : Importer.GetCollection()) {

            nlohmann::json obj = {
                    {"name", i.GetName()},
                    {"id", static_cast<uint64_t>(i.GetUUID())},
                    {"path", std::filesystem::relative(i.GetPath(), std::filesystem::current_path())}
            };
            v.push_back(obj);
        }
    }

    void Save(const std::filesystem::path& path)
    {
        std::ofstream ofs(path,std::ios::out);
        if(!ofs)
            throw std::runtime_error("Blyat");
        ofs << v.dump(4);
        ofs.close();
    }
};