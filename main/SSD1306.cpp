// SSD1306.cpp - OLED Display Driver Logic
#include "SSD1306.hpp"
#include <cstring>

namespace Clock {
bool SSD1306::init() {
    // Send initialization sequence commands over I2CBus
    clear();
    flush();
    return true;
}

void SSD1306::clear() {
    memset(buffer_, 0, sizeof(buffer_));
}

void SSD1306::setPixel(int x, int y, bool color) {
    if (x < 0 || x >= 128 || y < 0 || y >= 64) return;
    if (color) buffer_[x + (y / 8) * 128] |= (1 << (y % 8));
    else buffer_[x + (y / 8) * 128] &= ~(1 << (y % 8));
}

void SSD1306::flush() {
    // Transmit buffer_ via thread-safe I2CBus
}
}