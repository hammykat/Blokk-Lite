#pragma once

#include "miniaudio.h"
#include "raylib.h"

#include <robin_map.h>

#include <unordered_map>
#include <string>
#include <vector>
#include <filesystem>
#include <functional>
#include <memory>

#include "Types.hpp"
#include "coordinate_tree.hpp"

using ObjectID = uint32_t;

class GameObject;

struct ObjectIndexesData
{
    GameObject* ObjectInstance = nullptr;
    ObjectID collisionRect = UINT32_MAX;
    ObjectID position = UINT32_MAX;
    ObjectID velocity = UINT32_MAX;
    ObjectID animationData = UINT32_MAX;
};

struct AnimationData {
    ObjectID AnimationNumber = UINT32_MAX;
    ObjectID FrameNumber = UINT32_MAX;
};

class Engine
{

private:
    friend class GameObject;

    // Collisions
    const uint32_t CollisionChunkSize = 1024;
    std::vector<Rect> m_CollisionRectangles;
    CoordinateTree2D<std::unique_ptr<ObjectIndexesData>> m_ChunkData;

    // Sound
    ma_engine m_AudioEngine;
    std::unordered_map<std::string, uint32_t> m_SoundNames;
    std::vector<std::vector<ma_sound>> m_Sounds;
    std::unordered_map<uint32_t, uint32_t> m_SoundPoolSizes;
    std::unordered_map<uint32_t, uint32_t> m_CurrentInstanceIndex;

    // Animations
    // An animation is a list of frames (raylib textures). m_Animations[AnimationIndex][FrameIndex]
    // m_AnimationNames turns the name of an animation into its AnimationIndex
    tsl::robin_map<std::string, uint32_t> m_AnimationNames;
    std::vector<std::vector<Texture2D>> m_Animations; // Frames for each animation
    std::vector<uint32_t> m_AnimationSizes; // The number of frames in each animation
    std::vector<uint32_t> m_AnimationNumbers;
    std::vector<uint32_t> m_FrameNumbers;
    uint32_t m_AnimationCount = 0; // How many animations have been created

    // Object data
    std::vector<float> m_XVelocities;
    std::vector<float> m_YVelocities;
    std::vector<float> m_XPositions;
    std::vector<float> m_YPositions;

    // Index data
    std::vector<ObjectIndexesData> m_ObjectLocationData;
    uint32_t m_ObjectCount = 0;
    double m_DeltaTime = 0.0f;

    void RunCollisions();
    void RenderFrame();

    void AddObject(ObjectCreationData& creationData, GameObject* object);
    void RegisterCollisionRect(Rect& rect, GameObject* object);
    void RemoveObject();

public:
    Engine();
    ~Engine();

    void ExecuteFrame() 
    {
        m_DeltaTime = GetFrameTime();

        // Run collisions, update positions
        RunCollisions();

        // Render all objects
        BeginDrawing();
            RenderFrame();
        EndDrawing();
    }

    // Sounds ----------------------------------------

    void LoadSound(const std::filesystem::path& path, const std::string& name, uint32_t maxConcurrentInstances = 1);
    void PlaySoundEffect(const std::string& name, float volume = 100.0f);
    void StopSound(const std::string& name);
    void ClearSounds();

    // Animations --------------------------------------

    // Textures can only be loaded after raylib's InitWindow(), because they live on the GPU.

    // Creates an animation called 'name' and loads every file in 'paths' as a frame, in order
    void CreateAnimation(const std::vector<std::filesystem::path>& paths, const std::string& name);

    // Creates an animation called 'name' with no frames. Use AddAnimationFrame() to fill it
    void CreateEmptyAnimation(const std::string& name);

    // Loads the file at 'path' and adds it as the last frame of the animation called 'name'
    void AddAnimationFrame(const std::string& name, const std::filesystem::path& path);

    // Unloads every animation. Objects that were using an animation must be given a new one afterwards
    void ClearAnimations();

    // Returns the index of the animation called 'name', or UINT32_MAX if there is none
    [[nodiscard]] uint32_t GetAnimationIndex(const std::string& name) const;

    // Returns how many frames an animation has, or 0 if there is no animation at that index
    [[nodiscard]] uint32_t GetAnimationFrameCount(uint32_t animationIndex) const;

    [[nodiscard]] const std::vector<std::vector<Texture2D>>& GetAnimations() const { return m_Animations; }
};
