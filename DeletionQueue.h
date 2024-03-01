//
// Created by Dima on 01.03.2024.
//
#include "functional"
#include "vector"
#pragma once

class DeletionQueue
        {
    std::vector<std::function<void()>> StorageOfFunctions;

        public:

    void InputFunction(std::function<void()> Function)
    {
        StorageOfFunctions.emplace_back(std::move(Function));
    }

    void OutputFunction()
    {
        for(auto& func:StorageOfFunctions)
        {
            func();
        }
        StorageOfFunctions.clear();
    }
        };
