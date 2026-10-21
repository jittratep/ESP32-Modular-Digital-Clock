// TimeManager.hpp - POSIX System & NTP Sync Subsystem
#pragma once
#include "DS1307.hpp"
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
    void renderTime(SSD1306& display) { /* Render clock UI */ }
};
}