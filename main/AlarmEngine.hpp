// AlarmEngine.hpp - Alarm Comparator & Piezo Buzzer
#pragma once
#include "driver/gpio.h"

namespace Clock {
class AlarmEngine {
private:
    gpio_num_t buzzerPin_;
    uint8_t alarmHour_{7};
    uint8_t alarmMinute_{0};
    bool enabled_{false};
    bool ringing_{false};

public:
    explicit AlarmEngine(gpio_num_t pin) : buzzerPin_(pin) {}
    void init() { gpio_set_direction(buzzerPin_, GPIO_MODE_OUTPUT); }
    void checkAlarm(uint8_t h, uint8_t m) {
        if (enabled_ && h == alarmHour_ && m == alarmMinute_) ringing_ = true;
    }
};
}