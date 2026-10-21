// DS1307.cpp - DS1307 Implementation
#include "DS1307.hpp"

namespace Clock {
bool DS1307::init() {
    uint8_t secondsReg = 0;
    if (bus_.readRegs(I2C_ADDR, 0x00, &secondsReg, 1)) {
        // Clear CH (Clock Halt) bit if set
        if (secondsReg & 0x80) {
            bus_.writeReg(I2C_ADDR, 0x00, 0x00);
        }
        return true;
    }
    return false;
}

bool DS1307::getTime(DateTime& dt) {
    uint8_t raw[7] = {0}; // เพิ่ม = {0} เพื่อกำหนดค่าเริ่มต้น
    if (!bus_.readRegs(I2C_ADDR, 0x00, raw, 7)) return false;
    dt.second = bcd2dec(raw[0] & 0x7F);
    dt.minute = bcd2dec(raw[1]);
    dt.hour = bcd2dec(raw[2] & 0x3F);
    dt.dayOfWeek = bcd2dec(raw[3]);
    dt.dayOfMonth = bcd2dec(raw[4]);
    dt.month = bcd2dec(raw[5]);
    dt.year = 2000 + bcd2dec(raw[6]);
    return true;
}

bool DS1307::setTime(const DateTime& dt) {
    if (dt.second > 59 || dt.minute > 59 || dt.hour > 23 ||
        dt.dayOfWeek < 1 || dt.dayOfWeek > 7 ||
        dt.dayOfMonth < 1 || dt.dayOfMonth > 31 ||
        dt.month < 1 || dt.month > 12 || dt.year < 2000 || dt.year > 2099) {
        return false;
    }

    const uint8_t values[] = {
        dec2bcd(dt.second), dec2bcd(dt.minute), dec2bcd(dt.hour),
        dec2bcd(dt.dayOfWeek), dec2bcd(dt.dayOfMonth), dec2bcd(dt.month),
        dec2bcd(static_cast<uint8_t>(dt.year - 2000))
    };
    for (uint8_t reg = 0; reg < sizeof(values); ++reg) {
        if (!bus_.writeReg(I2C_ADDR, reg, values[reg])) return false;
    }
    return true;
}
}