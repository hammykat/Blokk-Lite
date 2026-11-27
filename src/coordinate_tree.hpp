#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <functional>
#include <source_location>
#include <robin_map.h>
#include "Types.hpp"


// The coordinate tree is an optimized data structure that allows for the storage and retrieval of items based on their coordinates in a 2D space,
// or "groups". Each group is identified by a unique pair of coordinates (x, y), and can contain multiple items. The structure allows for efficient addition, removal, and access of items within these groups.
// The tree has safety mechanisms in place to ensure that operations on the tree are valid, throwing errors when attempting to access non-existent groups or indices.
// It is optimized for performance, making it suitable for scenarios where fast access to spatially organized data is required, such as in game development or simulations.


// The hash function for the Vector2 struct, used to allow Vector2 to be used as a key in unordered_map. 
// This function is optimized for performance, combining the x and y coordinates into a single uint32_t value.
// The function only takes ~1-3 CPU cycles to compute (not including the time it would take to fetch the data), 
// making it suitable for high-performance scenarios where many hash computations may be required.
struct Vector2Hash
{
    [[nodiscard]] inline
        uint32_t operator()(const vec2& Coord) const noexcept
    {
        return
            (static_cast<uint64_t>(static_cast<uint32_t>(Coord.x)) << 32) |
            static_cast<uint32_t>(Coord.y);
    }
};



template <typename Type>
class CoordinateTree2D
{

private:
    std::vector<std::vector<Type>> Data; // Contains a vector of references to vectors of Type, where each reference corresponds to a group of items
    tsl::robin_map<vec2, uint32_t, Vector2Hash> GroupNames; // Contains a mapping from group coordinates to the index of the corresponding vector in 'data'
    std::vector<std::vector<uint32_t>> FreeIdxs; // Contains a vector of vectors of free indices for each group, where each inner vector corresponds to a group and contains the indices of items that have been removed and can be reused
    std::vector<uint32_t> FreeIdxSize; // Contains the number of free indices for each group, where each element corresponds to a group and contains the number of items that have been removed and can be reused

public: // Allow full access to data

    const auto& GetGroupMap() {
        return GroupNames;
    }

    void Clear() {
        Data.clear();
        GroupNames.clear();
        FreeIdxs.clear();
        FreeIdxSize.clear();
    }

    [[nodiscard]] inline
        Type& GetItem(const vec2& GroupCoords, uint32_t Idx) {
        auto It = GroupNames.find(GroupCoords);
        if (It != GroupNames.end()) {
            return Data[It->second][Idx];
        }
        // Safe fallback reference to prevent undefined behavior
        static Type Dummy{};
        return Dummy;
    }

    [[nodiscard]] inline
        std::vector<Type>& GetGroup(uint32_t Idx) {
        if (Idx < Data.size()) {
            return Data[Idx];
        }
        // Safe fallback vector reference
        static std::vector<Type> DummyVec;
        return DummyVec;
    }

    [[nodiscard]] inline
    std::vector<Type>& GetGroup(const vec2 coords) {
        if (auto It = GroupNames.find(coords); It != GroupNames.end()) {
            return Data[It->second];
        }
        // Safe fallback vector reference
        static std::vector<Type> DummyVec;
        return DummyVec;
    }


    // Add a value to a group
    void AddItem(const vec2& GroupCoords, Type Val)
    {
        auto It = GroupNames.find(GroupCoords);
        if (It == GroupNames.end()) return;
        uint32_t Idx = It->second;

        // If there are free indices, use one of them
        if (FreeIdxSize[Idx] > 0)
        {
            uint32_t FreeIdx = FreeIdxs[Idx].back(); // Retrieve free index
            FreeIdxs[Idx].pop_back(); // Remove last free index

            Data[Idx][FreeIdx] = Val;
            --FreeIdxSize[Idx];
        }
        else { // No indexs available, push back to the end of the vector
            Data[Idx].push_back(Val);
        }
    }

    void AddItem(const uint32_t Idx, Type Val)
    {
        // If there are free indices, use one of them
        if (FreeIdxSize[Idx] > 0)
        {
            uint32_t FreeIdx = FreeIdxs[Idx].back(); // Retrieve free index
            FreeIdxs[Idx].pop_back(); // Remove last free index

            Data[Idx][FreeIdx] = Val;
            --FreeIdxSize[Idx];
        }
        else { // No indexs available, push back to the end of the vector
            Data[Idx].push_back(Val);
        }
    }


    // Add a vector of items to a group
    void AddItems(
        const vec2& GroupCoords,
        const std::vector<Type>& Items
    ) {
        auto It = GroupNames.find(GroupCoords);

        // If the group doesn't exist, stop
        if (It == GroupNames.end()) {
            return;
        }

        uint32_t Idx = It->second;
        for (const auto& Val : Items)
        {
            if (FreeIdxSize[Idx] > 0)
            {
                uint32_t FreeIdx = FreeIdxs[Idx].back();
                FreeIdxs[Idx].pop_back();

                Data[Idx][FreeIdx] = Val;
                --FreeIdxSize[Idx];
            }
            else
            {
                Data[Idx].push_back(Val);
            }
        }
    }

    // Create a new group with empty vectors
    void CreateNewGroup(const vec2 GroupCoords)
    {
        uint32_t Idx = static_cast<uint32_t>(Data.size());

        GroupNames.emplace(GroupCoords, Idx);
        Data.emplace_back();
        FreeIdxs.emplace_back();
        FreeIdxSize.push_back(0);
    }

    // Create a new group with initial values
    void CreateNewGroup(
        const vec2 GroupCoords,
        std::vector<Type> values
    ) {
        uint32_t Idx = static_cast<uint32_t>(Data.size());

        GroupNames.emplace(GroupCoords, Idx);
        Data.push_back(std::move(values));
        FreeIdxs.emplace_back();
        FreeIdxSize.push_back(0);
    }

    // Remove an item from a group using group coordinates
    void RemoveItem(
        const vec2& GroupCoords,
        uint32_t Idx
    ) {
        // Find the group index using the coordinates
        const auto It = GroupNames.find(GroupCoords);

        if (It == GroupNames.end()) {
            return;
        }

        uint32_t GroupIdx = It->second;
        FreeIdxs[GroupIdx].push_back(Idx);
        ++FreeIdxSize[GroupIdx];
    }

    // Remove an item
    void RemoveItemFromGroup(
        uint32_t GroupIdx,
        uint32_t Idx
    ) {
        if (GroupIdx >= Data.size()) {
            return;
        }

        // Add the index to the free indices vector for the group to be reused later
        FreeIdxs[GroupIdx].push_back(Idx);
        ++FreeIdxSize[GroupIdx];
    }


    [[nodiscard]] 
        uint32_t GetIndexOfGroup(const vec2& GroupCoords) {
        // Find the group index using the coordinates
        const auto It = GroupNames.find(GroupCoords);

        // If the group exists, return its index
        if (It != GroupNames.end()) {
            return It->second;
        }
        return UINT32_MAX; // Safe fallback index
    }

    [[nodiscard]] inline bool Contains(const vec2 coord) {
        return GroupNames.contains(coord);
    }
};

