// SSD1306.cpp - OLED Display Driver Logic
#include "SSD1306.hpp"
#include <cstring>

namespace Clock {
namespace {
uint8_t fontColumn(char character, size_t column) {
    static constexpr uint8_t font[][5] = {
        {0x3E, 0x51, 0x49, 0x45, 0x3E}, // 0
        {0x00, 0x42, 0x7F, 0x40, 0x00}, // 1
        {0x42, 0x61, 0x51, 0x49, 0x46}, // 2
        {0x21, 0x41, 0x45, 0x4B, 0x31}, // 3
        {0x18, 0x14, 0x12, 0x7F, 0x10}, // 4
        {0x27, 0x45, 0x45, 0x45, 0x39}, // 5
        {0x3C, 0x4A, 0x49, 0x49, 0x30}, // 6
        {0x01, 0x71, 0x09, 0x05, 0x03}, // 7
        {0x36, 0x49, 0x49, 0x49, 0x36}, // 8
        {0x06, 0x49, 0x49, 0x29, 0x1E}, // 9
        {0x7E, 0x11, 0x11, 0x11, 0x7E}, // A
        {0x7F, 0x49, 0x49, 0x49, 0x36}, // B
        {0x3E, 0x41, 0x41, 0x41, 0x22}, // C
        {0x7F, 0x41, 0x41, 0x22, 0x1C}, // D
        {0x7F, 0x49, 0x49, 0x49, 0x41}, // E
        {0x7F, 0x09, 0x09, 0x09, 0x01}, // F
        {0x3E, 0x41, 0x49, 0x49, 0x7A}, // G
        {0x7F, 0x08, 0x08, 0x08, 0x7F}, // H
        {0x00, 0x41, 0x7F, 0x41, 0x00}, // I
        {0x20, 0x40, 0x41, 0x3F, 0x01}, // J
        {0x7F, 0x08, 0x14, 0x22, 0x41}, // K
        {0x7F, 0x40, 0x40, 0x40, 0x40}, // L
        {0x7F, 0x02, 0x0C, 0x02, 0x7F}, // M
        {0x7F, 0x04, 0x08, 0x10, 0x7F}, // N
        {0x3E, 0x41, 0x41, 0x41, 0x3E}, // O
        {0x7F, 0x09, 0x09, 0x09, 0x06}, // P
        {0x3E, 0x41, 0x51, 0x21, 0x5E}, // Q
        {0x7F, 0x09, 0x19, 0x29, 0x46}, // R
        {0x46, 0x49, 0x49, 0x49, 0x31}, // S
        {0x01, 0x01, 0x7F, 0x01, 0x01}, // T
        {0x3F, 0x40, 0x40, 0x40, 0x3F}, // U
        {0x1F, 0x20, 0x40, 0x20, 0x1F}, // V
        {0x3F, 0x40, 0x38, 0x40, 0x3F}, // W
        {0x63, 0x14, 0x08, 0x14, 0x63}, // X
        {0x07, 0x08, 0x70, 0x08, 0x07}, // Y
        {0x61, 0x51, 0x49, 0x45, 0x43}  // Z
    };

    if (character >= 'a' && character <= 'z') character -= 'a' - 'A';
    if (character >= '0' && character <= '9') return font[character - '0'][column];
    if (character >= 'A' && character <= 'Z') return font[10 + character - 'A'][column];

    switch (character) {
        case ':': {
            static constexpr uint8_t glyph[] = {0x00, 0x36, 0x36, 0x00, 0x00};
            return glyph[column];
        }
        case '.': {
            static constexpr uint8_t glyph[] = {0x00, 0x60, 0x60, 0x00, 0x00};
            return glyph[column];
        }
        case '-': {
            static constexpr uint8_t glyph[] = {0x08, 0x08, 0x08, 0x08, 0x08};
            return glyph[column];
        }
        case '/': {
            static constexpr uint8_t glyph[] = {0x20, 0x10, 0x08, 0x04, 0x02};
            return glyph[column];
        }
        default:
            return 0;
    }
}
}

bool SSD1306::init() {
    static constexpr uint8_t initCommands[] = {
        0xAE, 0xD5, 0x80, 0xA8, 0x3F, 0xD3, 0x00, 0x40, 0x8D, 0x14,
        0x20, 0x02, 0xA1, 0xC8, 0xDA, 0x12, 0x81, 0xCF, 0xD9, 0xF1,
        0xDB, 0x40, 0xA4, 0xA6, 0xAF
    };
    if (!sendCommands(initCommands, sizeof(initCommands))) return false;
    clear();
    return flush();
}

void SSD1306::clear() {
    std::memset(buffer_, 0, sizeof(buffer_));
}

void SSD1306::setPixel(int x, int y, bool color) {
    if (x < 0 || x >= 128 || y < 0 || y >= 64) return;
    if (color) buffer_[x + (y / 8) * 128] |= (1 << (y % 8));
    else buffer_[x + (y / 8) * 128] &= ~(1 << (y % 8));
}

bool SSD1306::sendCommands(const uint8_t* commands, size_t len) {
    uint8_t packet[sizeof(uint8_t) + 25];
    if (commands == nullptr || len == 0 || len > sizeof(packet) - 1) return false;
    packet[0] = 0x00;
    std::memcpy(packet + 1, commands, len);
    return bus_.write(addr_, packet, len + 1);
}

bool SSD1306::flush() {
    uint8_t dataPacket[129];
    for (uint8_t page = 0; page < 8; ++page) {
        const uint8_t pageCommands[] = {
            static_cast<uint8_t>(0xB0 | page), 0x00, 0x10
        };
        if (!sendCommands(pageCommands, sizeof(pageCommands))) return false;

        dataPacket[0] = 0x40;
        std::memcpy(dataPacket + 1, buffer_ + page * 128, 128);
        if (!bus_.write(addr_, dataPacket, sizeof(dataPacket))) return false;
    }
    return true;
}

void SSD1306::renderHeader(bool wifiConnected) {
    drawString(0, 0, "ESP32 CLOCK");
    drawString(84, 0, wifiConnected ? "NTP OK" : "NTP --");
}

void SSD1306::drawString(int x, int y, const char* str) {
    if (str == nullptr) return;

    for (size_t characterIndex = 0; str[characterIndex] != '\0'; ++characterIndex) {
        const char character = str[characterIndex];
        if (character != ' ') {
            for (size_t column = 0; column < 5; ++column) {
                const uint8_t pixels = fontColumn(character, column);
                for (size_t row = 0; row < 7; ++row) {
                    if ((pixels & (1U << row)) != 0) {
                        setPixel(x + static_cast<int>(characterIndex * 6 + column),
                                 y + static_cast<int>(row), true);
                    }
                }
            }
        }
    }
}
}