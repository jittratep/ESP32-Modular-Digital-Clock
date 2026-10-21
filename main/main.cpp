// main.cpp - ESP32 Digital Clock Application Entry Point
#include <stdio.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"
#include "esp_log.h"

#include "I2CBus.hpp"
#include "DS1307.hpp"
#include "SSD1306.hpp"
#include "ButtonManager.hpp"
#include "TimeManager.hpp"
#include "AlarmEngine.hpp"
#include "StopwatchEngine.hpp"

static const char* TAG = "MAIN_APP";

// Shared Drivers & System Objects
static Clock::I2CBus i2cBus(I2C_NUM_0, GPIO_NUM_21, GPIO_NUM_22);
static Clock::DS1307 rtcDriver(i2cBus);
static Clock::SSD1306 display(i2cBus, 0x3C);
static Clock::ButtonManager buttonMgr;
static Clock::TimeManager timeMgr(rtcDriver);
static Clock::AlarmEngine alarmEngine(GPIO_NUM_25);
static Clock::StopwatchEngine stopwatchEngine;

enum class ScreenMode : uint8_t { CLOCK, ALARM_SET, STOPWATCH, TIME_SET };

static void renderScreen(ScreenMode mode, bool ntpSynced, bool alarmEnabled,
                         uint8_t alarmHour, uint8_t alarmMinute) {
    display.clear();
    display.renderHeader(ntpSynced);

    switch (mode) {
        case ScreenMode::CLOCK: {
            Clock::DateTime dateTime{};
            if (!timeMgr.getTime(dateTime)) {
                display.drawString(0, 24, "RTC READ ERROR");
                break;
            }

            char text[24];
            snprintf(text, sizeof(text), "%02u:%02u:%02u",
                     static_cast<unsigned>(dateTime.hour),
                     static_cast<unsigned>(dateTime.minute),
                     static_cast<unsigned>(dateTime.second));
            display.drawString(0, 16, text);
            snprintf(text, sizeof(text), "%04u-%02u-%02u",
                     static_cast<unsigned>(dateTime.year),
                     static_cast<unsigned>(dateTime.month),
                     static_cast<unsigned>(dateTime.dayOfMonth));
            display.drawString(0, 32, text);
            display.drawString(0, 48, alarmEnabled ? "ALARM ON" : "ALARM OFF");
            break;
        }
        case ScreenMode::ALARM_SET: {
            char text[24];
            display.drawString(0, 16, "ALARM SETTING");
            snprintf(text, sizeof(text), "%02u:%02u",
                     static_cast<unsigned>(alarmHour),
                     static_cast<unsigned>(alarmMinute));
            display.drawString(0, 32, text);
            display.drawString(0, 48, alarmEnabled ? "STATUS ENABLED" : "STATUS DISABLED");
            display.drawString(0, 56, "SET ADD 5 MIN");
            break;
        }
        case ScreenMode::STOPWATCH: {
            const int64_t elapsedMs = stopwatchEngine.elapsedMicros() / 1000;
            const int64_t totalMinutes = elapsedMs / 60000;
            const int64_t seconds = (elapsedMs / 1000) % 60;
            const int64_t centiseconds = (elapsedMs % 1000) / 10;
            char text[24];
            display.drawString(0, 16, "STOPWATCH");
            snprintf(text, sizeof(text), "%02" PRId64 ":%02" PRId64 ".%02" PRId64,
                     totalMinutes, seconds, centiseconds);
            display.drawString(0, 24, text);
            display.drawString(0, 32, stopwatchEngine.isRunning() ? "RUNNING" : "PAUSED");
            for (size_t i = 0; i < stopwatchEngine.lapCount() && i < 3; ++i) {
                const int64_t lapMs = stopwatchEngine.lapMicros(i) / 1000;
                snprintf(text, sizeof(text), "LAP%u %02" PRId64 ".%02" PRId64,
                         static_cast<unsigned>(i + 1), lapMs / 1000, (lapMs % 1000) / 10);
                display.drawString(0, 40 + static_cast<int>(i) * 8, text);
            }
            break;
        }
        case ScreenMode::TIME_SET: {
            Clock::DateTime dateTime{};
            display.drawString(0, 16, "MANUAL TIME SET");
            if (timeMgr.getTime(dateTime)) {
                char text[16];
                snprintf(text, sizeof(text), "%02u:%02u:%02u",
                         static_cast<unsigned>(dateTime.hour),
                         static_cast<unsigned>(dateTime.minute),
                         static_cast<unsigned>(dateTime.second));
                display.drawString(0, 32, text);
            } else {
                display.drawString(0, 32, "RTC READ ERROR");
            }
            display.drawString(0, 48, "SET INC 1 MIN");
            display.drawString(0, 56, "MODE TO RETURN");
            break;
        }
    }
    if (!display.flush()) {
        ESP_LOGE(TAG, "Failed to update OLED display");
    }
}

extern "C" void app_main(void) {
    ESP_LOGI(TAG, "Initializing Modular C++ OOP Clock System...");

    // Spiral 1: Hardware Layer Init
    if (!i2cBus.init()) {
        ESP_LOGE(TAG, "Failed to initialize I2C bus");
        return;
    }
    if (!rtcDriver.init()) {
        ESP_LOGW(TAG, "RTC is unavailable; clock screen will show an error");
    }
    if (!display.init()) {
        ESP_LOGE(TAG, "Failed to initialize SSD1306 display");
        return;
    }

    // Spiral 2 & 3: Managers & Input
    timeMgr.initWifiNTP("iotexpo", "12345678");
    if (!buttonMgr.init()) {
        ESP_LOGE(TAG, "Failed to initialize buttons");
        return;
    }
    alarmEngine.init();

    // Spawn FreeRTOS Tasks
    xTaskCreatePinnedToCore([](void* p){ buttonMgr.taskLoop(); }, "vTaskButton", 3072, NULL, 5, NULL, 0);
    xTaskCreatePinnedToCore([](void* p){ timeMgr.taskNtpLoop(); }, "vTaskNTP", 6144, NULL, 1, NULL, 0);
    xTaskCreatePinnedToCore([](void* p){ alarmEngine.taskAlarmLoop(timeMgr); }, "vTaskAlarm", 2048, NULL, 4, NULL, 1);
    
    // Main Display Render Loop (10 FPS)
    ScreenMode mode = ScreenMode::CLOCK;
    bool alarmEnabled = true;
    uint8_t alarmHour = 7;
    uint8_t alarmMinute = 0;
    while (true) {
        Clock::ButtonEvent event{};
        while (xQueueReceive(buttonMgr.getQueue(), &event, 0) == pdTRUE) {
            switch (event.id) {
                case Clock::ButtonId::MODE:
                    mode = static_cast<ScreenMode>((static_cast<uint8_t>(mode) + 1) % 4);
                    break;
                case Clock::ButtonId::SET:
                    if (mode == ScreenMode::ALARM_SET) {
                        alarmMinute = static_cast<uint8_t>(alarmMinute + 5);
                        if (alarmMinute >= 60) {
                            alarmMinute = static_cast<uint8_t>(alarmMinute - 60);
                            alarmHour = static_cast<uint8_t>((alarmHour + 1) % 24);
                        }
                    } else if (mode == ScreenMode::STOPWATCH) {
                        if (stopwatchEngine.isRunning()) stopwatchEngine.pause();
                        else stopwatchEngine.start();
                    } else if (mode == ScreenMode::TIME_SET && !timeMgr.incrementTimeMinute()) {
                        ESP_LOGE(TAG, "Failed to advance RTC time by one minute");
                    }
                    break;
                case Clock::ButtonId::ALARM:
                    alarmEnabled = !alarmEnabled;
                    break;
                case Clock::ButtonId::LAP:
                    if (mode == ScreenMode::STOPWATCH) {
                        if (stopwatchEngine.isRunning()) stopwatchEngine.recordLap();
                        else stopwatchEngine.reset();
                    }
                    break;
            }
        }

        renderScreen(mode, timeMgr.isNtpSynced(), alarmEnabled, alarmHour, alarmMinute);
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}