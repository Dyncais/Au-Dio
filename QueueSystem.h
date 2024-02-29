
#include "vector"
#include "import.h"
#include "iostream"
#include "SDL3_mixer/SDL_mixer.h"
#include <unordered_map>

#include <algorithm>
#include <ranges>
#include <random>

#pragma once


class Queue
{

private:
    bool loop = false;
    using UUIDFromMusicBlock = MusicPlayer::UUID;
    using UUIDFromQueue = MusicPlayer::UUID;
    std::unordered_map<UUIDFromQueue, UUIDFromMusicBlock> mappedIds;

    std::vector<UUIDFromQueue> mainqueue;
    UUIDFromQueue currentMusic{0};
public:

    std::vector<std::pair<MusicPlayer::UUID, MusicPlayer::UUID>> getMainqueue() const
    {
        std::vector<std::pair<MusicPlayer::UUID, MusicPlayer::UUID>> result;
        result.reserve(mainqueue.size());
        for(auto internalId : mainqueue)
        {
            result.emplace_back(internalId, InternalIDToGlobalID(internalId));
        }
        return result;
    }

    bool isLoop() const
    {
        return loop;
    }


    Queue(const std::vector<MusicPlayer::UUID>& imported)
    {
        for(auto& uuid : imported)
        {
            AddToEnd(uuid);
        }
    }
    Queue() = default;

    MusicPlayer::UUID InternalIDToGlobalID(MusicPlayer::UUID internalId) const
    {
        return mappedIds.at(internalId);
    }

    MusicPlayer::UUID GetCurrentMusic()
    {
        return currentMusic == 0 ? MusicPlayer::UUID{0} : mappedIds[currentMusic];
    }

    void New_Crt_Mus(MusicPlayer::UUID idFromQueue)
    {
        currentMusic = idFromQueue;
    }

    void Next_Song()
    {
        if (mainqueue.empty())
        {
            currentMusic = {0};
            return;
        }

        if ((currentMusic == mainqueue[mainqueue.size() - 1]) and loop)
        {
            currentMusic = mainqueue[0];
            return;
        }


        if (currentMusic == mainqueue[mainqueue.size() - 1])
        {
            std::cout << "End";
            currentMusic = {0};
            return;
        }

        size_t i = 0;
        while (currentMusic != mainqueue[i])
        {
            i++;
        }
        currentMusic = mainqueue[i + 1];
    }

    void Prev_Song()
    {
        if (currentMusic == mainqueue[0])
        {
            return;
        }

        size_t i = 0;
        while (currentMusic != mainqueue[i])
        {
            i++;
        }
        currentMusic = mainqueue[i - 1];
    }

    void Randomize()
    {
        auto rng = std::default_random_engine {};
        std::shuffle(std::begin(mainqueue), std::end(mainqueue), rng);
    }

    void Swap(size_t prew,size_t nnew)
    {
        std::swap(mainqueue[prew], mainqueue[nnew]);
    }
    void ChangeLoop()
    {
        loop = !loop;
    }

    void Delete(MusicPlayer::UUID internalId)
    {
        mainqueue.erase(std::ranges::find(mainqueue, internalId));
        mappedIds.erase(internalId);
    }

    void AddToEnd(const MusicPlayer::UUID uuidFromMusicBlock)
    {
        MusicPlayer::UUID newId{};
        mappedIds[newId] = uuidFromMusicBlock;
        mainqueue.push_back(newId);
        //LogInfo("Added {0} to queue as {1}", uuidFromMusicBlock, newId);
        std::cout << std::format("Added {0} to queue as {1}", static_cast<uint64_t>(uuidFromMusicBlock), static_cast<uint64_t>(newId)) << '\n';
    }
};