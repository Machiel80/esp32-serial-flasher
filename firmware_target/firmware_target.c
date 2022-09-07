/* Copyright 2020 Espressif Systems (Shanghai) PTE LTD
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>
#include <string.h>
#include <sys/param.h>
#include "serial_io.h"
#include "esp_loader.h"
// #define LOG_LOCAL_LEVEL ESP_LOG_INFO // ESP_LOG_DEBUG, ESP_LOG_VERBOSE
#include "esp_log.h"
#include "firmware_target.h"
#include "main.h"

#define BOOTLOADER_ADDRESS  0x1000
#define PARTITION_ADDRESS   0x8000
#define OTA_ADDRESS         0xe000   // OTA_ADDRESS: See: https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/ota.html#ota-data-partition
#define APPLICATION_ADDRESS 0x10000  

extern const uint8_t  BOOTLOADER_BIN[];
extern const uint32_t BOOTLOADER_BIN_SIZE;
extern const uint8_t  PARTITION_TABLE_BIN[];
extern const uint32_t PARTITION_TABLE_BIN_SIZE;
extern const uint8_t  APPLICATION_BIN[];
extern const uint32_t APPLICATION_BIN_SIZE;

#ifdef OTA_ROM
extern const uint8_t  OTA_BIN[];
extern const uint32_t OTA_BIN_SIZE;
#endif

void get_binaries(target_chip_t target, target_binaries_t *bins) {
    
    bins->boot.data = BOOTLOADER_BIN;
    bins->boot.size = BOOTLOADER_BIN_SIZE;
    bins->boot.addr = BOOTLOADER_ADDRESS;
    bins->part.data = PARTITION_TABLE_BIN;
    bins->part.size = PARTITION_TABLE_BIN_SIZE;
    bins->part.addr = PARTITION_ADDRESS;
    bins->app.data  = APPLICATION_BIN;
    bins->app.size  = APPLICATION_BIN_SIZE;
    bins->app.addr  = APPLICATION_ADDRESS;
#ifdef OTA_ROM
    bins->ota.data  = OTA_BIN;
    bins->ota.size  = OTA_BIN_SIZE;
    bins->ota.addr  = OTA_ADDRESS;
#endif
}

esp_loader_error_t connect_to_target(uint32_t higher_baudrate) {
    static const char* TAG = "connect to target";
    esp_loader_connect_args_t connect_config = ESP_LOADER_CONNECT_DEFAULT();

    esp_loader_error_t err = esp_loader_connect(&connect_config);
    if (err != ESP_LOADER_SUCCESS) {
        ESP_LOGD(TAG,"cannot connect to target. Error: %u", err);
        return err;
    }
     ESP_LOGI(TAG,"connected to target");

    if (higher_baudrate && esp_loader_get_target() != ESP8266_CHIP) {
        err = esp_loader_change_baudrate(higher_baudrate);
        if (err == ESP_LOADER_ERROR_UNSUPPORTED_FUNC) {
            ESP_LOGD(TAG,"ESP8266 does not support change baudrate command");
            return err;
        } else if (err != ESP_LOADER_SUCCESS) {
            ESP_LOGD(TAG,"unable to change baud rate on target");
            return err;
        } else {
            err = loader_port_change_baudrate(higher_baudrate);
            if (err != ESP_LOADER_SUCCESS) {
                ESP_LOGD(TAG,"unable to change baud rate");
                return err;
            }
            ESP_LOGI(TAG,"baudrate changed");
        }
    }
    return ESP_LOADER_SUCCESS;
}

esp_loader_error_t flash_binary(const uint8_t *bin, size_t size, size_t address) {
    static const char* TAG = "flash binary";
    esp_loader_error_t err;
    static uint8_t payload[1024];
    const uint8_t *bin_addr = bin;

    ESP_LOGI(TAG,"erasing flash (this may take a while)...");
    err = esp_loader_flash_start(address, size, sizeof(payload));
    if (err != ESP_LOADER_SUCCESS) {
        ESP_LOGD(TAG,"erasing flash failed with error %d.\n", err);
        return err;
    }
    ESP_LOGI(TAG,"start programming");

    size_t binary_size = size;
    size_t written = 0;

    while (size > 0) {
        size_t to_read = MIN(size, sizeof(payload));
        memcpy(payload, bin_addr, to_read);

        err = esp_loader_flash_write(payload, to_read);
        if (err != ESP_LOADER_SUCCESS) {
            ESP_LOGD(TAG,"packet could not be written! error %d", err);
            return err;
        }

        size -= to_read;
        bin_addr += to_read;
        written += to_read;

        int progress = (int)(((float)written / binary_size) * 100);
        ESP_LOGD(TAG,"progress: %d %%", progress);
        fflush(stdout);
    };

    ESP_LOGI(TAG,"finished programming");

#if MD5_ENABLED
    err = esp_loader_flash_verify();
    if (err == ESP_LOADER_ERROR_UNSUPPORTED_FUNC) {
        ESP_LOGD(TAG,"ESP8266 does not support flash verify command");
        return err;
    } else if (err != ESP_LOADER_SUCCESS) {
        ESP_LOGD(TAG,"MD5 does not match. err: %d", err);
        return err;
    }
    ESP_LOGI(TAG,"flash verified");
#endif

    return ESP_LOADER_SUCCESS;
}