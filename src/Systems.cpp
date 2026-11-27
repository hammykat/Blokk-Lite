#define NOMINMAX

#include "Types.hpp"
#include "Engine.hpp"

#include "GameObject.hpp"

#include <xsimd/xsimd.hpp>
#include <algorithm>

struct UpdatePositionsKernel 
{
	template <typename Arch>
    void operator()(Arch arch, float* XVelocities, float* YVelocities, float* XPositions, float* YPositions, float delta, uint32_t count)
    {
        using Batch = xsimd::batch<float, Arch>;
        constexpr uint32_t inc = Batch::size;
        uint32_t i = 0;

        for (; i + inc <= count; i += inc) {
            Batch XPos = Batch::load_unaligned(XPositions + i);
            Batch YPos = Batch::load_unaligned(YPositions + i);

            Batch XVel = Batch::load_unaligned(XVelocities + i);
            Batch YVel = Batch::load_unaligned(YVelocities + i);

            Batch DeltaTime = Batch(delta);

            XPos += XVel * DeltaTime;
            YPos += YVel * DeltaTime;

            XPos.store_unaligned(XPositions + i);
            YPos.store_unaligned(YPositions + i);
        }

        // Scalar remainder
        for (; i < count; ++i) {
            XPositions[i] += XVelocities[i] * delta;
            YPositions[i] += YVelocities[i] * delta;
        }
	}
};


void UpdateFrameNumbers(uint32_t* FrameNumbers, uint32_t* AnimationNumbers, uint32_t* AnimationSizes, uint32_t count) {
    for (uint32_t i = 0; i < count; i++) {
        FrameNumbers[i] = (FrameNumbers[i] + 1) % AnimationSizes[AnimationNumbers[i]];
    }
}


// Draws a bunch of objects with raylib.
// Object i is drawn using the animation data AnimData[i] at the position (XPositions[i], YPositions[i]).
// 'Animations' is the Engine's list of animations (see Engine::GetAnimations()).
// Objects without an animation (or with a frame that doesn't exist) are skipped.
// This doesn't call BeginDrawing() or EndDrawing(), the caller does that.
void RenderObjects(
    const std::vector<std::vector<Texture2D>>& Animations,
    const uint32_t* AnimationNumbers,
    const uint32_t* FrameNumbers,
    const float* XPositions,
    const float* YPositions,
    uint32_t count
) {
    const uint32_t animationCount = static_cast<uint32_t>(Animations.size());

    for (uint32_t i = 0; i < count; i++) {
        const AnimationData& animation = {AnimationNumbers[i], FrameNumbers[i]};

        // No animation set (UINT32_MAX) or an animation that doesn't exist
        if (animation.AnimationNumber >= animationCount)
            continue;

        const std::vector<Texture2D>& frames = Animations[animation.AnimationNumber];

        // No frame set (UINT32_MAX) or a frame that doesn't exist
        if (animation.FrameNumber >= frames.size())
            continue;

        DrawTextureV(frames[animation.FrameNumber], Vector2{ XPositions[i], YPositions[i] }, WHITE);
    }
}


void RunCollisionsForObjects(
    std::vector<float>& XPositions,
    std::vector<float>& YPositions,
    std::vector<float>& XVelocities,
    std::vector<float>& YVelocities,
    std::vector<Rect>& CollisionRects,
    std::vector<std::reference_wrapper<ObjectIndexesData>>& IndexData
) {
    uint32_t count = IndexData.size();
    
    for (uint32_t i = 0; i < count; i++) 
    {
        ObjectIndexesData& data = IndexData[i];

        // No collision set
        if (data.collisionRect == UINT32_MAX) 
            continue;

        Rect& rect = CollisionRects[data.collisionRect];
        float& x = XPositions[data.position];
        float& y = YPositions[data.position];
        float ox = x + rect.x; float oy = y + rect.y; // Ofsetted position
        float w = rect.w; float h = rect.h;
        GameObject* obj = data.ObjectInstance;

        for (uint32_t u = i + 1; u < count; u++) {
            ObjectIndexesData& otherData = IndexData[u];

            // No collision set
            if (otherData.collisionRect == UINT32_MAX)
                continue;

            Rect& rect2 = CollisionRects[otherData.collisionRect];
            float& x2 = XPositions[otherData.position];
            float& y2 = YPositions[otherData.position];
            float ox2 = x2 + rect2.x; float oy2 = y2 + rect2.y;
            float w2 = rect2.w; float h2 = rect2.h;
            GameObject* obj2 = otherData.ObjectInstance;

            if (
                ox + w > ox2 &&
                ox < ox2 + w2 &&
                oy + h > oy2 &&
                oy < oy2 + h2
            ) {
                // Handle a collision!
                vec2 v = { XVelocities[i], YVelocities[i] };
                vec2 v2 = { XVelocities[u], YVelocities[u] };

                // Calculate penetration depths on both axes
                float overlapX = std::min(ox + w, ox2 + w2) - std::max(ox, ox2);
                float overlapY = std::min(oy + h, oy2 + h2) - std::max(oy, oy2);

                // Resolve along the axis of least penetration
                if (overlapX < overlapY) {
                    // Separate horizontally
                    float halfOverlap = overlapX * 0.5f;
                    if (ox < ox2) {
                        x -= halfOverlap;
                        x2 += halfOverlap;
                    }
                    else {
                        x += halfOverlap;
                        x2 -= halfOverlap;
                    }
                }
                else {
                    // Separate vertically
                    float halfOverlap = overlapY * 0.5f;
                    if (oy < oy2) {
                        y -= halfOverlap;
                        y2 += halfOverlap;
                    }
                    else {
                        y += halfOverlap;
                        y2 -= halfOverlap;
                    }
                }

                obj->RunCollisionFunction(obj2);
                obj2->RunCollisionFunction(obj);
            }
        }
    }
}


// Updates the chunks of objects in the world and arranges them for collision checks
// Hardcoded to chunk sizes of 512, uses SIMD
struct UpdateCollisionChunksKernel
{
    template <typename Arch>
    void operator()(Arch arch,
        float* XPositions, float* YPositions,
        CoordinateTree2D<std::reference_wrapper<ObjectIndexesData>>& chunkContainer,
        std::vector<ObjectIndexesData>& IndexesData,
        uint32_t count
    ) {
        std::vector<float> chunkXResults(count);
        std::vector<float> chunkYResults(count);

        using FloatBatch = xsimd::batch<float, Arch>;
        using IntBatch = xsimd::batch<int, Arch>;

        constexpr uint32_t inc = FloatBatch::size;
        uint32_t i = 0;

        for (; i + inc <= count; i += inc) {
            FloatBatch XPos = FloatBatch::load_unaligned(XPositions + i);
            FloatBatch YPos = FloatBatch::load_unaligned(YPositions + i);

            IntBatch IntXPos = xsimd::to_int(XPos);
            IntBatch IntYPos = xsimd::to_int(YPos);
            
            IntBatch ChunkX = IntXPos >> 9;
            IntBatch ChunkY = IntYPos >> 9;
            
            ChunkX.store_unaligned(chunkXResults.data() + i);
            ChunkY.store_unaligned(chunkYResults.data() + i);
        }

        // Scalar remainder
        for (; i < count; ++i) {
            chunkXResults[i] = XPositions[i] >> 9;
            chunkYResults[i] = YPositions[i] >> 9;
        }

        for (uint32_t i = 0; i < count; i++) {
            const vec2 coord = { chunkXResults[i], chunkYResults[i] };
            
            if (!chunkContainer.Contains(coord))
                chunkContainer.CreateNewGroup(coord);

            chunkContainer.AddItem(coord, IndexesData[i]);
        }
    }
};

namespace {
    auto UpdatePositionsDispatcher = xsimd::dispatch(UpdatePositionsKernel{});
    auto UpdateCollisionChunksDispatcher = xsimd::dispatch(UpdateCollisionChunksKernel{});
}

void Engine::RenderFrame() 
{
    // Increment objects frame numbers
    UpdateFrameNumbers(
        m_FrameNumbers.data(),
        m_AnimationNumbers.data(),
        m_AnimationSizes.data(),
        m_FrameNumbers.size()
    );
    
    // Render all the objects
    RenderObjects(
        m_Animations,
        m_AnimationNumbers.data(),
        m_FrameNumbers.data(),
        m_XPositions.data(),
        m_YPositions.data(),
        m_AnimationNumbers.size()
    );
}


// Chunks it checks collisions with
// [ ][ ][x]
// [ ][x][x]
// [x][x][x]
const std::vector<vec2> ChunksToRunCollisionsMap = {
    {0, 0}, // The chunk
    {-1, -1}, // Bottom left
    {0, -1}, // Bottom middle
    {1, -1}, // Bottom right
    {1, 0}, // Middle right
    {1, 1} // Top left
};

inline void GetChunksToRunCollisions(const vec2& tcoord, std::vector<vec2>& outCoords) {
    outCoords.clear();
    outCoords.reserve(ChunksToRunCollisionsMap.size());

    for (const vec2& offset : ChunksToRunCollisionsMap) {
        outCoords.push_back({ tcoord.x + offset.x, tcoord.y + offset.y });
    }
}

void Engine::RunCollisions() 
{
    // Update positions based on velocities
    UpdatePositionsDispatcher(
        m_XVelocities.data(),
        m_YVelocities.data(),
        m_XPositions.data(),
        m_YPositions.data(),
        m_DeltaTime,
        m_XVelocities.size()
    );

    // Update object chunks
    m_ChunkData.Clear(); 
    UpdateCollisionChunksDispatcher(
        m_XPositions.data(),
        m_YPositions.data(),
        m_ChunkData,
        m_ObjectLocationData, 
        m_XVelocities.size()
    );
    
    std::vector<vec2> currentCoordsToCheck;
    std::vector<std::reference_wrapper<ObjectIndexesData>> currentData;

    auto& GroupMap = m_ChunkData.GetGroupMap();
    for (const auto& [chunkCoord, idx] : GroupMap) {
        GetChunksToRunCollisions(chunkCoord, currentCoordsToCheck);

        currentData.clear();
        currentData = m_ChunkData.GetGroup(idx);
        uint32_t firstSize = currentData.size();

        RunCollisionsForObjects(
            m_XPositions, m_YPositions,
            m_XVelocities, m_YVelocities,
            m_CollisionRectangles,
            currentData
        );

        for (const vec2 coord : currentCoordsToCheck) {
            if (coord == chunkCoord) continue;
            if (!m_ChunkData.Contains(coord)) continue;

            auto& data = m_ChunkData.GetGroup(coord);
            currentData.resize(firstSize);
            currentData.reserve(firstSize + data.size());
            currentData.insert(currentData.end(), data.begin(), data.end());

            RunCollisionsForObjects(
                m_XPositions, m_YPositions,
                m_XVelocities, m_YVelocities,
                m_CollisionRectangles,
                currentData
            );
        }
    }
}


