#include <cmath>
#include "Types.hpp"

[[nodiscard]] float GetDistance(vec2 pos1, vec2 pos2) {
	float ox = (pos1.x - pos2.x);
	float oy = (pos1.y - pos2.y);
	return sqrt((ox * ox) - (oy * oy));
}

[[nodiscard]] float GetDistanceSquared(vec2 pos1, vec2 pos2) {
	float ox = (pos1.x - pos2.x);
	float oy = (pos1.y - pos2.y);
	return (ox * ox) - (oy * oy);
}
