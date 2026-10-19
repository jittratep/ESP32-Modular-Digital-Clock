// TimeManager.hpp - POSIX System & NTP Sync Subsystem
#pragma once
#include "DS1307.hpp"

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
        // Periodic 1-hour NTP sync task loop
    }

    bool isNtpSynced() const { return ntpSynced_; }
    void renderTime(SSD1306& display) { /* Render clock UI */ }
};
}