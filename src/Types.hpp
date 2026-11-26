
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
};


struct Rect {
	int x, y; // Top left 
	int w, h; // Dimensions
};


