// TimeManager.hpp - POSIX System & NTP Sync Subsystem
#pragma once
#include "DS1307.hpp"
#include "SSD1306.hpp"
#include <cstdio>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

namespace Clock {
class TimeManager {
private:
    DS1307& rtc_;
    bool ntpSynced_{false};

public:
    explicit TimeManager(DS1307& rtc) : rtc_(rtc) {}

    void initWifiNTP(const char* ssid, const char* pass) {
        // Connect to Wi-Fi 'iotexpo' and trigger esp_sntp
    }

    void taskNtpLoop() {
        while (true) {
            // โค้ดสำหรับดึงเวลาจากอินเทอร์เน็ต (NTP) จะใส่ไว้ตรงนี้
            vTaskDelay(pdMS_TO_TICKS(1000)); // หน่วงเวลา 1 วินาที
        }
    }

    bool isNtpSynced() const { return ntpSynced_; }

    bool getTime(DateTime& dateTime) { return rtc_.getTime(dateTime); }

    bool incrementTimeMinute() {
        DateTime dateTime{};
        if (!rtc_.getTime(dateTime)) return false;
        if (dateTime.hour > 23 || dateTime.minute > 59 ||
            dateTime.dayOfWeek < 1 || dateTime.dayOfWeek > 7 ||
            dateTime.month < 1 || dateTime.month > 12 ||
            dateTime.dayOfMonth < 1 ||
            dateTime.dayOfMonth > daysInMonth(dateTime.month, dateTime.year) ||
            dateTime.year < 2000 || dateTime.year > 2099) {
            return false;
        }

        if (++dateTime.minute == 60) {
            dateTime.minute = 0;
            if (++dateTime.hour == 24) {
                dateTime.hour = 0;
                dateTime.dayOfWeek = static_cast<uint8_t>(dateTime.dayOfWeek % 7 + 1);
                if (++dateTime.dayOfMonth > daysInMonth(dateTime.month, dateTime.year)) {
                    dateTime.dayOfMonth = 1;
                    if (++dateTime.month > 12) {
                        dateTime.month = 1;
                        ++dateTime.year;
                        if (dateTime.year > 2099) return false;
                    }
                }
            }
        }
        return rtc_.setTime(dateTime);
    }

    void renderTime(SSD1306& display) {
        DateTime dateTime{};
        if (!rtc_.getTime(dateTime)) {
            display.drawString(0, 24, "RTC READ ERROR");
            return;
        }
        char timeText[9];
        std::snprintf(timeText, sizeof(timeText), "%02u:%02u:%02u",
                      static_cast<unsigned>(dateTime.hour),
                      static_cast<unsigned>(dateTime.minute),
                      static_cast<unsigned>(dateTime.second));
        display.drawString(0, 24, timeText);
    }

private:
    static uint8_t daysInMonth(uint8_t month, uint16_t year) {
        static constexpr uint8_t days[] = {
            31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
        };
        if (month == 2 && (year % 4 == 0)) return 29;
        return days[month - 1];
    }
};
}