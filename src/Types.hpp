#pragma once

#include <cstdint>
#include <string>

template <typename Container>
struct Handle {
	Container* data = nullptr;
	uint32_t idx = UINT32_MAX;

	// Default
	Handle() = default;

	Handle(Container& _data, uint32_t _idx) {
		data = &_data;
		idx = _idx;
	}

	inline void Set(Container& _data, uint32_t _idx) {
		data = &_data;
		idx = _idx;
	}
	inline void SetIdx(uint32_t _idx) {
		idx = _idx;
	}

	[[nodiscard]] inline bool IsValid() {
		return !(data == nullptr || idx == UINT32_MAX);
	}

	[[nodiscard]] inline auto& get() const {
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


struct ObjectCreationData {
	const vec2 position = { 0, 0 }, velocity = { 0, 0 };
	const uint32_t frameNumber = UINT32_MAX;
	const std::string animationName{ };
};


