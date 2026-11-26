
#include "miniaudio.h"

#include <unordered_map>
#include <string>
#include <vector>
#include <filesystem>

#include "Types.hpp"
#include "coordinate_tree.hpp"

using ObjectID = uint32_t;

struct ObjectIndexesData 
{
    GameObject* ObjectInstance = nullptr;
    ObjectID collisionRect = UINT32_MAX;
    ObjectID position{} = UINT32_MAX;
    ObjectID velocity{} = UINT32_MAX;
    ObjectID isVisible{} = UINT32_MAX;
    ObjectID animationData{} = UINT32_MAX;
};

struct AnimationData {
    ObjectID AnimationNumber = UINT32_MAX;
    ObjectID FrameNumber = UINT32_MAX;
};

class GameObject;

class Engine 
{

private:

    // Collisions
    const uint32_t CollisionChunkSize = 1024;
    std::vector<Rect> m_CollisionRectangles;
    CoordinateTree2D<std::reference_wrapper<ObjectIndexesData>> Chunks;

    // Sound
    ma_engine m_AudioEngine;
    std::unordered_map<std::string, uint32_t> m_SoundNames;
    std::vector<std::vector<ma_sound>> m_Sounds;
    std::unordered_map<uint32_t, uint32_t> m_SoundPoolSizes;
    std::unordered_map<uint32_t, uint32_t> m_CurrentInstanceIndex;

    // Object data
    std::vector<float> m_XVelocities;
    std::vector<float> m_YVelocities;
    std::vector<float> m_XPositions;
    std::vector<float> m_YPositions;

    std::vector<bool> m_IsVisible;
    std::vector<AnimationData> m_AnimationData;

    // Index data
    std::vector<ObjectIndexesData> m_ObjectLocationData;

public:
    Engine();
    ~Engine();

    void LoadSound(const std::filesystem::path& path, const std::string& name, uint32_t maxConcurrentInstances = 1);
    void PlaySoundEffect(const std::string& name, float volume = 100.0f);
    void StopSound(const std::string& name);
    void ClearSounds();
};
