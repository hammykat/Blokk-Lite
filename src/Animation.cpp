#include "Engine.hpp"

#include <format>
#include <stdexcept>

namespace {

// Loads one frame from a file. Throws if raylib can't load it.
Texture2D LoadFrame(const std::filesystem::path& path)
{
    Texture2D texture = LoadTexture(path.string().c_str());

    // raylib gives back a texture with id 0 when loading fails
    if (texture.id == 0) {
        auto err_str = std::format("Blokk Error: Failed to load animation frame at path: {}", path.string());
        throw std::runtime_error(err_str);
    }

    return texture;
}

// Textures can't be created without a window (OpenGL context)
void RequireWindow(const char* functionName)
{
    if (!IsWindowReady()) {
        auto err_str = std::format("Blokk Error: {}() was called before InitWindow()! Textures can only be loaded after the window exists.", functionName);
        throw std::runtime_error(err_str);
    }
}

} // namespace


// Creates an animation and loads every frame, in order
void Engine::CreateAnimation(const std::vector<std::filesystem::path>& paths, const std::string& name)
{
    if (m_AnimationNames.contains(name))
        return; // Already exists

    RequireWindow("Engine::CreateAnimation");

    // Load into a temporary vector first, so a bad file doesn't leave a half-made animation behind
    std::vector<Texture2D> frames;
    frames.reserve(paths.size());

    try {
        for (const std::filesystem::path& path : paths) {
            frames.push_back(LoadFrame(path));
        }
    }
    catch (...) {
        for (Texture2D& frame : frames) {
            UnloadTexture(frame);
        }
        throw;
    }

    // Add it all at once
    m_AnimationNames[name] = m_AnimationCount;
    m_Animations.push_back(std::move(frames));
    m_AnimationSizes.push_back(frames.size());
    m_AnimationCount++;
}


// Creates an animation without any frames
void Engine::CreateEmptyAnimation(const std::string& name)
{
    if (m_AnimationNames.contains(name))
        return; // Already exists

    m_AnimationNames[name] = m_AnimationCount;
    m_Animations.emplace_back();
    m_AnimationSizes.push_back(0);
    m_AnimationCount++;
}


// Loads a file and adds it to the end of an existing animation
void Engine::AddAnimationFrame(const std::string& name, const std::filesystem::path& path)
{
    auto it = m_AnimationNames.find(name);
    if (it == m_AnimationNames.end()) {
        auto err_str = std::format("Animation '{}' does not exist in Engine::AddAnimationFrame()", name);
        throw std::runtime_error(err_str);
    }

    RequireWindow("Engine::AddAnimationFrame");

    m_Animations[it->second].push_back(LoadFrame(path));
    m_AnimationSizes[it->second]++;
}


// Unloads every frame of every animation and resets everything
void Engine::ClearAnimations()
{
    // If the window is already closed, raylib has freed the textures for us, and unloading them again would be invalid
    if (IsWindowReady()) {
        for (auto& frames : m_Animations) {
            for (Texture2D& frame : frames) {
                UnloadTexture(frame);
            }
        }
    }

    m_Animations.clear();
    m_AnimationNames.clear();
    m_AnimationSizes.clear();
    m_AnimationCount = 0;
}


uint32_t Engine::GetAnimationIndex(const std::string& name) const
{
    auto it = m_AnimationNames.find(name);
    if (it == m_AnimationNames.end())
        return UINT32_MAX;

    return it->second;
}


uint32_t Engine::GetAnimationFrameCount(uint32_t animationIndex) const
{
    if (animationIndex >= m_AnimationCount)
        return 0;

    return static_cast<uint32_t>(m_AnimationSizes[animationIndex]);
}
