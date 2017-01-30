#include "gslX680_touchscreen.h"

#include <initializer_list>

#include <Arduino.h>
#include <Wire.h>

#include "firmware.h"

using namespace std;

const uint8_t TS_ADDR = 0x40;

GSLx680Touchscreen::GSLx680Touchscreen(TwoWire &i2c) : i2c(i2c) {}

void GSLx680Touchscreen::begin(int8_t INTR) {
	if ((intr = INTR) >= 0) {
		pinMode(intr, INPUT);
	}
	gslx680_reset();
	const uint8_t *fw_begin = GSLX680_FW;
	const uint8_t *fw_end = GSLX680_FW + GSLX680_FW_bytes;
	for (auto p = fw_begin; p != fw_end; p += 5) {
		ts_write(p, 5, false);
	}
	ts_write({0xe0, 0x00});
	gslx680_reset();
	ts_write({0xe0, 0x00});
}

namespace {
uint16_t extr16(const uint8_t *twoBytes) {
	return twoBytes[0] + (uint16_t(twoBytes[1]) << 8);
}
} // namespace

int GSLx680Touchscreen::getNumTouches() {
	// intr sometimes misses when fingers are removed, so always read again
	// when at least one is sensed
	if ((intr < 0) || digitalRead(intr) || (numTouches > 0)) {
		uint8_t buf[24];
		if (ts_read(0x80, 24, buf)) {
			numTouches = min(MAX_TOUCHES, buf[0]);
			uint8_t offs = 4;
			for (int n = 0; n < numTouches; ++n, offs += 4) {
				uint16_t fy = extr16(buf + offs + 2);
				int finger = fy >> 12;
				touches[finger][0] = extr16(buf + offs);
				touches[finger][1] = fy & ((uint16_t(1) << 12) - 1);
			}
		}
		else {
			numTouches = 0;
		}
	}
	return numTouches;
}

// first byte is register, following are data
bool GSLx680Touchscreen::ts_write(const uint8_t *bytes, size_t length, bool slow) {
	i2c.beginTransmission(TS_ADDR);
	i2c.write(bytes, length);
	auto result = i2c.endTransmission(true); // stop
	if (slow) {
		delay(20);
	}
	return (result == 0);
}

inline bool GSLx680Touchscreen::ts_write(initializer_list<uint8_t> bytes) {
	return ts_write(&*bytes.begin(), bytes.size());
}

bool GSLx680Touchscreen::ts_read(uint8_t addr, size_t count, uint8_t *dest) {
	i2c.beginTransmission(TS_ADDR);
	i2c.write(addr);
	if (0 != i2c.endTransmission(true)) { // stop
		return false;
	}
	auto recv = i2c.requestFrom(TS_ADDR, count, true);
	if (count != recv) { // stop
		// flush input buffer
		while (i2c.available() > 0) {
			i2c.read();
		}
		return false;
	}
	while (count-- != 0) {
		*dest++ = uint8_t(i2c.read());
	}
	return true;
}

void GSLx680Touchscreen::gslx680_reset() {
	ts_write({0xe0, 0x88});
	ts_write({0xe4, 0x04});
	ts_write({0xbc, 0x00, 0x00, 0x00, 0x00});
}
