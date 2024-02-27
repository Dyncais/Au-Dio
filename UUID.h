//
// Created by Dima on 24.02.2024.
//

#pragma once


#include <cstdint>
#include <string>
namespace MusicPlayer
{

    class UUID
    {
    public:
        UUID();

        constexpr UUID(uint64_t uuid)
                : m_UUID(uuid)
        {}

        UUID(const UUID &) = default;

        operator uint64_t() const
        { return m_UUID; }

    private:
        uint64_t m_UUID;
    };
}

    namespace std
    {
        template<>
        struct hash<MusicPlayer::UUID>
        {
            std::size_t operator()(const MusicPlayer::UUID &uuid) const
            {
                return hash<uint64_t>()((uint64_t) uuid);
            }
        };

    }

