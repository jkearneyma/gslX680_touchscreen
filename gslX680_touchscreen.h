#pragma once

#include <cstdint>
#include <cstdlib>
#include <initializer_list>
#include <utility>

class TwoWire;

class GSLx680Touchscreen {
public:

	static const int MAX_TOUCHES = 10;

	GSLx680Touchscreen(TwoWire &i2c);

	void begin(int8_t INTR = -1);

	// this must be called and return non-zero for getTouch() to return valid data
	int getNumTouches();

	std::pair<int /*x*/, int /*y*/> getTouch(int whichFinger = 0) const {
		return { touches[whichFinger][0], touches[whichFinger][1] };
	}

private:
	TwoWire &i2c;
	int8_t intr = -1;
	int numTouches = 0;
	uint16_t touches[MAX_TOUCHES][2];

	bool ts_write(const uint8_t *bytes, size_t length, bool slow = true);
	inline bool ts_write(std::initializer_list<uint8_t> bytes);
	bool ts_read(uint8_t addr, size_t count, uint8_t *dest);
	void gslx680_reset();
};

