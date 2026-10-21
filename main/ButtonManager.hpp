// ButtonManager.hpp - GPIO ISR Switch Debouncer
#pragma once
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"

namespace Clock {
enum class ButtonId { MODE, SET, ALARM, LAP };
enum class PressType { SHORT, LONG };

struct ButtonEvent {
    ButtonId id;
    PressType type;
};

class ButtonManager {
private:
    static constexpr size_t BUTTON_COUNT = 4;
    const gpio_num_t pins_[BUTTON_COUNT] = {
        GPIO_NUM_18, GPIO_NUM_19, GPIO_NUM_4, GPIO_NUM_5
    };
    const ButtonId buttonIds_[BUTTON_COUNT] = {
        ButtonId::MODE, ButtonId::SET, ButtonId::ALARM, ButtonId::LAP
    };
    bool pressed_[BUTTON_COUNT]{};
    QueueHandle_t eventQueue_{nullptr};

public:
    bool init() {
        eventQueue_ = xQueueCreate(10, sizeof(ButtonEvent));
        if (eventQueue_ == nullptr) {
            ESP_LOGE("BUTTON", "Failed to create button event queue");
            return false;
        }

        gpio_config_t config = {};
        for (const gpio_num_t pin : pins_) {
            config.pin_bit_mask |= (1ULL << pin);
        }
        config.mode = GPIO_MODE_INPUT;
        config.pull_up_en = GPIO_PULLUP_ENABLE;
        config.pull_down_en = GPIO_PULLDOWN_DISABLE;
        config.intr_type = GPIO_INTR_DISABLE;
        const esp_err_t result = gpio_config(&config);
        if (result != ESP_OK) {
            ESP_LOGE("BUTTON", "Failed to configure button GPIOs: %s", esp_err_to_name(result));
            return false;
        }
        return true;
    }

    void taskLoop() {
        while (true) {
            for (size_t i = 0; i < BUTTON_COUNT; ++i) {
                const bool isPressed = gpio_get_level(pins_[i]) == 0;
                if (isPressed == pressed_[i]) continue;

                vTaskDelay(pdMS_TO_TICKS(30));
                if ((gpio_get_level(pins_[i]) == 0) != isPressed) continue;

                pressed_[i] = isPressed;
                if (isPressed) {
                    const ButtonEvent event{buttonIds_[i], PressType::SHORT};
                    if (xQueueSend(eventQueue_, &event, 0) != pdTRUE) {
                        ESP_LOGW("BUTTON", "Button event queue is full");
                    }
                }
            }
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }

    QueueHandle_t getQueue() const { return eventQueue_; }
};
}