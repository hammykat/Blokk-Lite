#include <vector>
#include <string>

#include "Types.hpp"

using FloatHandle = Handle<std::vector<float>>;
using uIntHandle = Handle<std::vector<uint32_t>>;

using UpdateFunctionPtr = void(GameObject::*)(float delta);
using CollisionFunctionPtr = void(GameObject::*)(GameObject& Object);

class GameObject
{

private:

	FloatHandle XVelocity;
	FloatHandle YVelocity;

	FloatHandle XPosition;
	FloatHandle YPosition;

	uIntHandle FrameNumber;
	uIntHandle AnimationNumber;

	std::string CurrentAnimation;

	UpdateFunctionPtr UpdateFunction;
	CollisionFunctionPtr CollisionFunction;
	bool UpdateFunctionSet;
	bool CollisionFunctionSet;


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

	void SetFrameNumber(uint32_t num) {
		FrameNumber.get() = num;
	}

	void SetAnimation(std::string name) {
		CurrentAnimation = name;
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
