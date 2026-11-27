#include "Engine.hpp"
#include "GameObject.hpp"

void Engine::AddObject(ObjectCreationData& creationData, GameObject* object) 
{
	// Velocity
	auto& vel = creationData.velocity;
	m_XVelocities.push_back(vel.x);
	m_YVelocities.push_back(vel.y);

	// Position
	auto& pos = creationData.position;
	m_XPositions.push_back(pos.x);
	m_YPositions.push_back(pos.y);

	// Animation data
	m_AnimationNumbers.push_back(GetAnimationIndex(creationData.animationName));
	m_FrameNumbers.push_back(creationData.frameNumber);

	// Counter
	m_ObjectCount++;

	// Set object vars -----------------------------------
	object->EngineInstance = this;
	uint32_t idx = m_XPositions.size() - 1;
	
	object->XPosition.Set(m_XPositions, idx);
	object->YPosition.Set(m_YPositions, idx);

	object->XVelocity.Set(m_XVelocities, idx);
	object->YVelocity.Set(m_YVelocities, idx);
	
	object->FrameNumber.Set(m_FrameNumbers, idx);
	object->AnimationNumber.Set(m_AnimationNumbers, idx);
}


void Engine::RegisterCollisionRect(Rect& rect, GameObject* object) 
{
	uint32_t idx = m_CollisionRectangles.size();
	object->EngineIndexesData.get().collisionRect = idx;
	object->CollisionRect.Set(m_CollisionRectangles, idx);
	m_CollisionRectangles.push_back(rect);
}
