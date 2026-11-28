#include "Engine.hpp"
#include "GameObject.hpp"

#include <iostream>
#include <format>

// Constructor (initializes the AudioEngine)
Engine::Engine()
{
    // Initialize the miniaudio engine with default settings
    if (ma_engine_init(nullptr, &m_AudioEngine) != MA_SUCCESS) {
        throw std::runtime_error("Blokk Error: Failed to initialize audio engine!");
    }

    // Set this engine instance as the active one for handles/objects
    GameObject::EngineInstance = this;
}

// Destructor (Uninitializes the AudioEngine, cleans up all the sounds)
Engine::~Engine()
{
    // Clean up all loaded sound assets 
    ClearSounds();

    // Unload all animation textures (must happen before the window closes)
    ClearAnimations();

    // Uninitialize the engine itself
    ma_engine_uninit(&m_AudioEngine);
}

// Loads a sound from a filepath
void Engine::LoadSound(const std::filesystem::path& path, const std::string& name, uint32_t maxConcurrentInstances)
{
    if (m_SoundNames.contains(name) || maxConcurrentInstances == 0)
        return; // Already loaded or doesn't have to load

    uint32_t idx = static_cast<uint32_t>(m_Sounds.size());
    m_SoundNames[name] = idx;

    m_Sounds.emplace_back();
    std::vector<ma_sound>& soundInstances = m_Sounds[m_SoundNames[name]];
    soundInstances.resize(maxConcurrentInstances);

    ma_uint32 flags = MA_SOUND_FLAG_DECODE;

    // Load the first sound
    if (ma_sound_init_from_file(&m_AudioEngine, path.string().c_str(), flags, nullptr, nullptr, &soundInstances[0]) != MA_SUCCESS) {
        auto err_str = std::format("Blokk Error: Failed to load audio file at path: {}", path.string());
        throw std::runtime_error(err_str);
    }

    // Copy the sound into the other slots
    for (uint32_t i = 1; i < maxConcurrentInstances; i++) {
        ma_sound_init_copy(&m_AudioEngine, &soundInstances[0], flags, nullptr, &soundInstances[i]);
    }

    // Add it all at once
    m_SoundPoolSizes[m_SoundNames[name]] += maxConcurrentInstances;
}


// Plays a sound effect, at a specific volume
void Engine::PlaySoundEffect(const std::string& name, float volume)
{
    // Check if it exists
    auto it = m_SoundNames.find(name);
    if (it == m_SoundNames.end()) {
        auto err_str = std::format("Sound '{}' does not exist in Engine::PlaySoundEffect()", name);
        throw std::runtime_error(err_str);
    }

    uint32_t idx = it->second;
    ma_sound* sound = nullptr;

    // Get the sound using the index
    uint32_t& soundIdx = m_CurrentInstanceIndex[idx];
    sound = &m_Sounds[idx][soundIdx];

    // Play the sound
    ma_sound_set_volume(sound, volume / 100.0f); // Set the sound
    ma_sound_stop(sound); // Stop the sound if playing
    ma_sound_seek_to_pcm_frame(sound, 0); // Restart 
    ma_sound_start(sound);

    // Advance the index for the next time it needs to get played
    soundIdx = (soundIdx + 1) % m_SoundPoolSizes[idx];
}


// Stops all of a sound's instances
void Engine::StopSound(const std::string& name)
{
    auto it = m_SoundNames.find(name);
    if (it == m_SoundNames.end())
        return;

    uint32_t idx = it->second;
    std::vector<ma_sound>& soundInstances = m_Sounds[idx];

    // Loop through every instance and stop
    for (uint32_t i = 0, count = m_SoundPoolSizes[idx]; i < count; i++) {
        ma_sound_stop(&soundInstances[i]);
    }
}


// Clears all sounds and resets everything
void Engine::ClearSounds()
{
    // Loops through every instance for every sound and uninitialize
    for (auto& soundInstances : m_Sounds) {
        for (auto& sound : soundInstances) {
            ma_sound_uninit(&sound);
        }
    }

    // Clear all arrays
    m_Sounds.clear();

    m_SoundNames.clear();
    m_SoundPoolSizes.clear();
    m_CurrentInstanceIndex.clear();
}
