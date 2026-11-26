#define NOMINMAX

#include "Types.hpp"
#include "Engine.hpp"

#include "GameObject.hpp"

#include <xsimd/xsimd.hpp>
#include <algorithm>

struct UpdatePositionsKernel 
{
	template <typename Arch>
	void operator()(Arch arch, float* XVelocities, float* YVelocities, float* XPositions, float* YPositions, uint32_t count) 
    {
        using Batch = xsimd::batch<float, Arch>;
        constexpr uint32_t inc = Batch::size;
        uint32_t i = 0;

        for (; i + inc <= count; i += inc) {
            Batch XPos = Batch::load_unaligned(XPositions + i);
            Batch YPos = Batch::load_unaligned(YPositions + i);

            Batch XVel = Batch::load_unaligned(XVelocities + i);
            Batch YVel = Batch::load_unaligned(YVelocities + i);

            XPos += XVel;
            YPos += YVel;

            XPos.store_unaligned(XPositions + i);
            YPos.store_unaligned(YPositions + i);
        }

        // Scalar remainder
        for (; i < count; ++i) {
            XPositions[i] += XVelocities[i];
            YPositions[i] += YVelocities[i];
        }
	}
};


void UpdateFrameNumbers(uint32_t* FrameNumbers, uint32_t* AnimationNumbers, uint32_t* AnimationSizes, uint32_t count) {
    for (uint32_t i = 0; i < count; i++) {
        FrameNumbers[i] = (FrameNumbers[i] + 1) % AnimationSizes[AnimationNumbers[i]];
    }
}


void RunCollisions(
    std::vector<float>& XPositions,
    std::vector<float>& YPositions,
    std::vector<float>& XVelocities,
    std::vector<float>& YVelocities,
    std::vector<Rect>& CollisionRects,
    std::vector<ObjectIndexesData>& IndexData
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


struct UpdateCollisionChunksKernel
{
    template <typename Arch>
    void operator()(Arch arch, float* XPositions, float* YPositions, CoordinateTree2D<std::reference_wrapper<ObjectIndexesData>>& chunkContainer, uint32_t count)
    {
        std::vector<float> chunkXResults(count);
        std::vector<float> chunkYResults(count);

        using Batch = xsimd::batch<float, Arch>;
        constexpr uint32_t inc = Batch::size;
        uint32_t i = 0;

        for (; i + inc <= count; i += inc) {
            Batch XPos = Batch::load_unaligned(XPositions + i);
            Batch YPos = Batch::load_unaligned(YPositions + i);
            
            Batch ChunkX = XPos >> 10;
            Batch ChunkY = YPos >> 10;

            ChunkX.store_unaligned(chunkXResults.data() + i);
            ChunkY.store_unaligned(chunkYResults.data() + i);
        }

        // Scalar remainder
        for (; i < count; ++i) {
            chunkXResults[i] = XPositions[i] >> 10;
            chunkYResults[i] = YPositions[i] >> 10;
        }

        for (uint32_t i = 0; i < count; i++) {
            const vec2 coord = { chunkXResults[i], chunkYResults[i] };
            
            if (!chunkContainer.Contains(coord))
                chunkContainer.CreateNewGroup(coord);

            uint32_t idx = chunkContainer.GetIndexOfGroup(coords);
            chunkContainer.AddItem(idx)
        }
    }
};


