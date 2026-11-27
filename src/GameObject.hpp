#pragma once

#include <vector>
#include <string>
#include <format>
#include <stdexcept>

#include "Types.hpp"
#include "Engine.hpp"

class GameObject;

using FloatHandle = Handle<std::vector<float>>;
using uIntHandle = Handle<std::vector<uint32_t>>;
using AnimationDataHandle = Handle<std::vector<AnimationData>>;

using UpdateFunctionPtr = void(GameObject::*)(float delta);
using CollisionFunctionPtr = void(GameObject::*)(GameObject& Object);

class GameObject
{

private:

	FloatHandle XVelocity;
	FloatHandle YVelocity;

	FloatHandle XPosition;
	FloatHandle YPosition;

	AnimationDataHandle AnimationData;
	std::string CurrentAnimation;

	UpdateFunctionPtr UpdateFunction;
	CollisionFunctionPtr CollisionFunction;
	bool UpdateFunctionSet;
	bool CollisionFunctionSet;

	Engine* EngineInstance = nullptr; // The Engine that owns this object (set by the Engine when it creates the object)

public:
	friend class Engine;

	void SetVelocity(vec2 vel) {
		XVelocity.get() = vel.x;
		YVelocity.get() = vel.y;
	}

	[[nodiscard]] inline vec2 GetVelocity() {
		return vec2(XVelocity.get(), YVelocity.get());
	}

	void SetPosition(vec2 pos) {
		XPosition.get() = pos.x;
		YPosition.get() = pos.y;
	}

	[[nodiscard]] inline vec2 GetPosition() {
		return vec2(XPosition.get(), YPosition.get());
	}

	// Animations -------------------------------------------------------------

	// Sets the frame of the current animation. Does nothing if the frame doesn't exist in the current animation
	// (or if the object has no animation yet)
	void SetFrameNumber(uint32_t num) {
		if (EngineInstance == nullptr)
			return;

		// A frame number must be less than the number of frames in the animation
		if (num >= EngineInstance->GetAnimationFrameCount(AnimationData.get().AnimationNumber))
			return;

		AnimationData.get().FrameNumber = num;
	}

	// Sets the animation by the name it was given in the Engine (see Engine::CreateAnimation)
	void SetAnimation(const std::string& name) {
		if (EngineInstance == nullptr)
			return;

		// Look up the name in the Engine to get the animation's index
		const uint32_t animationIndex = EngineInstance->GetAnimationIndex(name);
		if (animationIndex == UINT32_MAX) {
			auto err_str = std::format("Animation '{}' does not exist in GameObject::SetAnimation()", name);
			throw std::runtime_error(err_str);
		}

		AnimationData.get().AnimationNumber = animationIndex;
		CurrentAnimation = name;

		// If the current frame doesn't exist in the new animation, go back to the first one
		if (AnimationData.get().FrameNumber >= EngineInstance->GetAnimationFrameCount(animationIndex))
			AnimationData.get().FrameNumber = 0;
	}

	void RunCollisionFunction(GameObject* obj) {
		if (CollisionFunctionSet) {
			(this->*CollisionFunction)(*obj);
		}
	}

	void RunUpdateFunction(float delta) {
		if (UpdateFunctionSet) {
			(this->*UpdateFunction)(delta);
		}
	}
};
