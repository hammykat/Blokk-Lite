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
using RectHandle = Handle<std::vector<Rect>>;
using IndexesDataHandle = Handle<std::vector<ObjectIndexesData>>;

using UpdateFunctionPtr = void(GameObject::*)(float delta);
using CollisionFunctionPtr = void(GameObject::*)(GameObject& Object);

class GameObject
{

private:
	inline static Engine* EngineInstance = nullptr; // The Engine that owns this object (set by the Engine when it creates the object)

	FloatHandle XVelocity;
	FloatHandle YVelocity;

	FloatHandle XPosition;
	FloatHandle YPosition;

	uIntHandle FrameNumber;
	uIntHandle AnimationNumber;

	RectHandle CollisionRect;
	IndexesDataHandle EngineIndexesData;

	UpdateFunctionPtr UpdateFunction;
	CollisionFunctionPtr CollisionFunction;
	bool UpdateFunctionSet;
	bool CollisionFunctionSet;

public:
	friend class Engine;

	GameObject(ObjectCreationData& creationData) {
		EngineInstance->AddObject(creationData, this);
	}

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

	// Collisions -------------------------------------

	void SetCollisionRect(Rect& rect) {
		if (CollisionRect.IsValid()) {
			// Set the var
			CollisionRect.get() = rect;	
		}
		else {
			// Tell engine to register
			EngineInstance->RegisterCollisionRect(rect, this);
		}
	}

	// Animations -------------------------------------------------------------

	// Sets the frame of the current animation. Does nothing if the frame doesn't exist in the current animation
	// (or if the object has no animation yet)
	void SetFrameNumber(uint32_t num) {
		if (EngineInstance == nullptr)
			return;

		// A frame number must be less than the number of frames in the animation
		if (num >= EngineInstance->GetAnimationFrameCount(AnimationNumber.get()))
			return;

		FrameNumber.get() = num;
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

		AnimationNumber.get() = animationIndex;

		// If the current frame doesn't exist in the new animation, go back to the first one
		if (FrameNumber.get() >= EngineInstance->GetAnimationFrameCount(animationIndex))
			FrameNumber.get() = 0;
	}

	// Custom functions -------------------------------------------

	// Sets the custom update function for this game object
	void SetUpdateFunction(UpdateFunctionPtr func) {
		UpdateFunction = func;
		UpdateFunctionSet = (func != nullptr);
	}

	// Sets the custom collision callback function for this game object
	void SetCollisionFunction(CollisionFunctionPtr func) {
		CollisionFunction = func;
		CollisionFunctionSet = (func != nullptr);
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
