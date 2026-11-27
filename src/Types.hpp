#pragma once


#include <cstdint>

template <typename Container>
struct Handle {
	Container* data;
	uint32_t idx;

	Handle(uint32_t idx) {
		idx = _idx;
	}

	[[nodiscard]] inline auto& get() {
		return (*data)[idx];
	}
};

struct vec2 {
	float x, y;

	bool operator==(const vec2& t) const {
		return x == t.x && y == t.y;
	}

	void operator+=(const vec2 t) {
		x += t.x; y += t.y;
	}
};


struct Rect {
	int x, y; // Top left 
	int w, h; // Dimensions
};


