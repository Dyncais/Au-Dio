
#include "vector"
#include "import.h"
#include "iostream"
#include "SDL3_mixer/SDL_mixer.h"
#include <unordered_map>

#include <algorithm>
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

    const std::vector<MusicPlayer::UUID> &getMainqueue() const
    {
        return mainqueue;
    }

    bool isLoop() const
    {
        return loop;
    }


    Queue(const std::vector<MusicPlayer::UUID>& imported)
    {
        for(auto& uuid : imported)
        {
            MusicPlayer::UUID newId{};
            mappedIds[newId] = uuid;
            mainqueue.push_back(newId);
        }
    }

    MusicPlayer::UUID InternalIDToGlobalID(MusicPlayer::UUID internalId)
    {
        return mappedIds[internalId];
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

    void Delete(size_t i)
    {
        mainqueue.erase(mainqueue.begin() + i);
    }

    void AddToEnd(const MusicPlayer::UUID uuidFromMusicBlock)
    {
        MusicPlayer::UUID newId{};
        mappedIds[newId] = uuidFromMusicBlock;
        mainqueue.push_back(newId);
    }
};