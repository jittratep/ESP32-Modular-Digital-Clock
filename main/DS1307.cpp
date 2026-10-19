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
    uint8_t raw[7];
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
}