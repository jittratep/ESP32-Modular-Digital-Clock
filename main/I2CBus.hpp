// I2CBus.hpp - Thread-Safe ESP-IDF v6 I2C Master Wrapper
#pragma once
#include <cstddef>
#include "driver/i2c_master.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

namespace Clock {
class I2CBus {
private:
    static constexpr size_t MAX_DEVICES = 4;
    static constexpr uint32_t I2C_TIMEOUT_MS = 1000;

    struct DeviceEntry {
        uint8_t address{0};
        i2c_master_dev_handle_t handle{nullptr};
    };

private:
    i2c_port_t port_;
    gpio_num_t sdaPin_;
    gpio_num_t sclPin_;
    i2c_master_bus_handle_t busHandle_{nullptr};
    SemaphoreHandle_t mutex_{nullptr};
    DeviceEntry devices_[MAX_DEVICES]{};

    i2c_master_dev_handle_t getDevice(uint8_t address) {
        for (auto& device : devices_) {
            if (device.handle != nullptr && device.address == address) {
                return device.handle;
            }
        }

        for (auto& device : devices_) {
            if (device.handle == nullptr) {
                i2c_device_config_t deviceConfig = {};
                deviceConfig.dev_addr_length = I2C_ADDR_BIT_LEN_7;
                deviceConfig.device_address = address;
                deviceConfig.scl_speed_hz = 100000;
                if (i2c_master_bus_add_device(busHandle_, &deviceConfig, &device.handle) != ESP_OK) {
                    return nullptr;
                }
                device.address = address;
                return device.handle;
            }
        }
        return nullptr;
    }

public:
    I2CBus(i2c_port_t port, gpio_num_t sda, gpio_num_t scl)
        : port_(port), sdaPin_(sda), sclPin_(scl) {
        mutex_ = xSemaphoreCreateMutex();
    }

    bool init() {
        if (mutex_ == nullptr) return false;

        i2c_master_bus_config_t bus_config = {};
        bus_config.i2c_port = port_;
        bus_config.sda_io_num = sdaPin_;
        bus_config.scl_io_num = sclPin_;
        bus_config.clk_source = I2C_CLK_SRC_DEFAULT;
        bus_config.glitch_ignore_cnt = 7;
        return i2c_new_master_bus(&bus_config, &busHandle_) == ESP_OK;
    }

    bool write(uint8_t devAddr, const uint8_t* data, size_t len) {
        if (data == nullptr || len == 0 || busHandle_ == nullptr ||
            xSemaphoreTake(mutex_, pdMS_TO_TICKS(I2C_TIMEOUT_MS)) != pdTRUE) {
            return false;
        }

        i2c_master_dev_handle_t device = getDevice(devAddr);
        const esp_err_t result = device == nullptr
            ? ESP_ERR_NOT_FOUND
            : i2c_master_transmit(device, data, len, I2C_TIMEOUT_MS);
        xSemaphoreGive(mutex_);
        return result == ESP_OK;
    }

    bool writeReg(uint8_t devAddr, uint8_t reg, uint8_t data) {
        const uint8_t payload[] = {reg, data};
        if (busHandle_ == nullptr ||
            xSemaphoreTake(mutex_, pdMS_TO_TICKS(I2C_TIMEOUT_MS)) != pdTRUE) {
            return false;
        }

        i2c_master_dev_handle_t device = getDevice(devAddr);
        const esp_err_t result = device == nullptr
            ? ESP_ERR_NOT_FOUND
            : i2c_master_transmit(device, payload, sizeof(payload), I2C_TIMEOUT_MS);
        xSemaphoreGive(mutex_);
        return result == ESP_OK;
    }

    bool readRegs(uint8_t devAddr, uint8_t startReg, uint8_t* buffer, size_t len) {
        if (buffer == nullptr || len == 0 || busHandle_ == nullptr ||
            xSemaphoreTake(mutex_, pdMS_TO_TICKS(I2C_TIMEOUT_MS)) != pdTRUE) {
            return false;
        }

        i2c_master_dev_handle_t device = getDevice(devAddr);
        const esp_err_t result = device == nullptr
            ? ESP_ERR_NOT_FOUND
            : i2c_master_transmit_receive(device, &startReg, 1, buffer, len, I2C_TIMEOUT_MS);
        xSemaphoreGive(mutex_);
        return result == ESP_OK;
    }
};
}