// StopwatchEngine.hpp - Split Timing Engine
#pragma once
#include "esp_timer.h"

namespace Clock {
class StopwatchEngine {
private:
    int64_t startTime_{0};
    int64_t elapsed_{0};
    bool running_{false};
    int64_t laps_[5];
    size_t lapCount_{0};

public:
    void start() { if(!running_) { startTime_ = esp_timer_get_time() - elapsed_; running_ = true; } }
    void pause() { if(running_) { elapsed_ = esp_timer_get_time() - startTime_; running_ = false; } }
    void reset() { running_ = false; elapsed_ = 0; lapCount_ = 0; }
    void recordLap() { if(lapCount_ < 5) laps_[lapCount_++] = elapsed_; }
};
}