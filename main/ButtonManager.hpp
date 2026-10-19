// ButtonManager.hpp - GPIO ISR Switch Debouncer
#pragma once
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

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
        // Configure GPIO 18, 19, 4, 5 with pull-up and ISR interrupts
    }

    void taskLoop() {
        // Poll GPIO states & push events into queue
    }

    QueueHandle_t getQueue() const { return eventQueue_; }
};
}