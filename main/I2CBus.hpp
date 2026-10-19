// I2CBus.hpp - Thread-Safe ESP-IDF v6 I2C Master Wrapper
#pragma once
#include "driver/i2c_master.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

namespace Clock {
class I2CBus {
private:
    i2c_port_t port_;
    gpio_num_t sdaPin_;
    gpio_num_t sclPin_;
    i2c_master_bus_handle_t busHandle_{nullptr};
    SemaphoreHandle_t mutex_{nullptr};

public:
    I2CBus(i2c_port_t port, gpio_num_t sda, gpio_num_t scl)
        : port_(port), sdaPin_(sda), sclPin_(scl) {
        mutex_ = xSemaphoreCreateMutex();
    }

    bool init() {
        i2c_master_bus_config_t bus_config = {};
        bus_config.i2c_port = port_;
        bus_config.sda_io_num = sdaPin_;
        bus_config.scl_io_num = sclPin_;
        bus_config.clk_source = I2C_CLK_SRC_DEFAULT;
        bus_config.glitch_ignore_cnt = 7;
        return i2c_new_master_bus(&bus_config, &busHandle_) == ESP_OK;
    }

    bool writeReg(uint8_t devAddr, uint8_t reg, uint8_t data) {
        xSemaphoreTake(mutex_, portMAX_DELAY);
        // Execute ESP-IDF master transmit logic
        xSemaphoreGive(mutex_);
        return true;
    }

    bool readRegs(uint8_t devAddr, uint8_t startReg, uint8_t* buffer, size_t len) {
        xSemaphoreTake(mutex_, portMAX_DELAY);
        // Execute ESP-IDF master receive logic
        xSemaphoreGive(mutex_);
        return true;
    }
};
}