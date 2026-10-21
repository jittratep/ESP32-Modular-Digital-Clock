// ButtonManager.hpp - GPIO ISR Switch Debouncer
#pragma once
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h" // เพิ่มสำหรับใช้คำสั่ง ESP_LOGI

namespace Clock {
enum class ButtonId { MODE, SET, ALARM, LAP };
enum class PressType { SHORT, LONG };

struct ButtonEvent {
    ButtonId id;
    PressType type;
};

class ButtonManager {
private:
    QueueHandle_t eventQueue_;

public:
    void init() {
        eventQueue_ = xQueueCreate(10, sizeof(ButtonEvent));
        
        // กำหนดขา GPIO 18 (MODE), 19 (SET), 4 (ALARM), 5 (LAP) เป็น Input Pull-up
        const gpio_num_t pins[] = {GPIO_NUM_18, GPIO_NUM_19, GPIO_NUM_4, GPIO_NUM_5};
        for (int i = 0; i < 4; i++) {
            gpio_set_direction(pins[i], GPIO_MODE_INPUT);
            gpio_set_pull_mode(pins[i], GPIO_PULLUP_ONLY); // เปิด Pull-up เพื่อให้สถานะปกติเป็น 1
        }
    }

    void taskLoop() {
        while (true) {
            // เช็กปุ่ม MODE (GPIO 18) - ถ้าถูกกดค่าจะโดนดึงลงกราวด์เป็น 0 (LOW)
            if (gpio_get_level(GPIO_NUM_18) == 0) {
                ESP_LOGI("BUTTON", "MODE (GPIO18) Pressed!");
                vTaskDelay(pdMS_TO_TICKS(250)); // Debounce ป้องกันการกดซ้ำรัวๆ
            }
            
            // เช็กปุ่ม SET (GPIO 19)
            if (gpio_get_level(GPIO_NUM_19) == 0) {
                ESP_LOGI("BUTTON", "SET/INC (GPIO19) Pressed!");
                vTaskDelay(pdMS_TO_TICKS(250));
            }

            // เช็กปุ่ม ALARM (GPIO 4)
            if (gpio_get_level(GPIO_NUM_4) == 0) {
                ESP_LOGI("BUTTON", "ALARM (GPIO4) Pressed!");
                vTaskDelay(pdMS_TO_TICKS(250));
            }

            // เช็กปุ่ม LAP (GPIO 5)
            if (gpio_get_level(GPIO_NUM_5) == 0) {
                ESP_LOGI("BUTTON", "LAP (GPIO5) Pressed!");
                vTaskDelay(pdMS_TO_TICKS(250));
            }

            vTaskDelay(pdMS_TO_TICKS(50)); // หน่วงเวลา 50ms เพื่อคืนทรัพยากรให้ CPU
        }
    }

    QueueHandle_t getQueue() const { return eventQueue_; }
};
}