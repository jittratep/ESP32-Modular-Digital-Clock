// main.cpp - ESP32 Digital Clock Application Entry Point
#include <stdio.h>
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

extern "C" void app_main(void) {
    ESP_LOGI(TAG, "Initializing Modular C++ OOP Clock System...");

    // Spiral 1: Hardware Layer Init
    i2cBus.init();
    rtcDriver.init();
    display.init();

    // Spiral 2 & 3: Managers & Input
    timeMgr.initWifiNTP("iotexpo", "12345678");
    buttonMgr.init();
    alarmEngine.init();

    // Spawn FreeRTOS Tasks
    xTaskCreatePinnedToCore([](void* p){ buttonMgr.taskLoop(); }, "vTaskButton", 3072, NULL, 5, NULL, 0);
    xTaskCreatePinnedToCore([](void* p){ timeMgr.taskNtpLoop(); }, "vTaskNTP", 6144, NULL, 1, NULL, 0);
    xTaskCreatePinnedToCore([](void* p){ alarmEngine.taskAlarmLoop(timeMgr); }, "vTaskAlarm", 2048, NULL, 4, NULL, 1);
    
    // Main Display Render Loop (10 FPS)
    while (true) {
        display.clear();
        // Render UI based on active mode state machine
        display.renderHeader(timeMgr.isNtpSynced());
        timeMgr.renderTime(display);
        display.flush();
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}